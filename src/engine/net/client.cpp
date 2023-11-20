#if !_SERVER

#include "client.h"
#include "data/address.h"
#include "data/host.h"
#include "data/peer.h"
#include "messages/server_welcome.h"
#include "player.h"
#include "data/packet.h"
#include "engine/game/local_player.h"
#include "engine/input/listener_game.h"
#include "messages/client_input.h"
#include "state.h"
#include "../game/match.h"
#include "snapshottable.h"
#include "../game_instance.h"
#include "game/cheat_manager.h"
#include "snapshot.h"
#include "../menus/basic_menu.h"
//#include "../system/terminal/terminal.h"

#if _DEV
extern KString CCOM_ServerQuit(const KString& val)
{
	if (KNetInterface_Client* iface = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		iface->RequestServerQuit();
	return "";
}
#endif

KString CCOM_ServerMap(const KString& val)
{
	if (KNetInterface_Client* c = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		c->CallServerMap(val);

	return "";
}

KNetInterface_Client::KNetInterface_Client()
{
	InitEnet();
	KCheatManager::ClearPersistentCheats();
	InterfaceType = ENetInterfaceType::Client;
	Host = std::make_unique<KNetHost>(1);
	if (GetGameMatch()) GetGameMatch()->InitNet();
}

KNetInterface_Client::~KNetInterface_Client()
{
	bIsShuttingDown = true;
	if (GetGameMatch() && !GetGameMatch()->ReplayReader.IsPlaying())
	{
		ClearConnectedPlayers();
		Host->Shutdown(Server.get());
	}
}

void KNetInterface_Client::AttemptConnection(class KNetAddress addr)
{
	Server = Host->InitConnection(&addr);
	if (Host->FinalizeConnection())
	{
		AttemptResult = EConnectionAttemptResult::Success;
	}
	else
	{
		AttemptResult = EConnectionAttemptResult::Failed;
		Server->ResetPeer();
	}
}

KString KNetInterface_Client::GetServerIP() const
{
	return Server->GetAddress().GetString();
}

/*
KNetPlayer* KNetInterface_Client::GetPlayerByIndex(u8 index)
{
	return &ConnectedPlayers[index];
}*/

void KNetInterface_Client::ClearPlayerByIndex(u8 index)
{
	if (ConnectedPlayers.contains(index))
		delete ConnectedPlayers[index];
		
	ConnectedPlayers.erase(index);
}

KNetPlayer* KNetInterface_Client::CreatePlayerAtIndex(u8 index)
{
	KNetPlayer* p = new KNetPlayer;
	ConnectedPlayers[index] = p;
	p->ClassID = KSnapshot::GetPlayerClassID();
	if (GetNetInterface()) GetNetInterface()->FlagPendingScoreUpdate();
	return p;
}

void KNetInterface_Client::AddPlayerToMap(KNetPlayer* player)
{
	if (KNetPlayer* p = ConnectedPlayers[player->OwningPlayerIndex])
	  if (p != player)
		delete p;

	// copy raw value into map
	ConnectedPlayers[player->OwningPlayerIndex] = player;
	//ConnectedPlayers[player->OwningPlayerIndex]->PlayerName = player->PlayerName;
	// player pointer passed to this function will be deleted soon
}

void KNetInterface_Client::AdjustTiming()
{
	KNetPlayer* player = GetLocalNetPlayer();
	if (!player || !GetGameMatch()/* || !GetNetState() || !player->IsReceivingState()*/) return;

	// we just made a hard adjustment
	/*if (player->ClientData.BruteForceFrameCorrect != 0) 
	{
		NetDilationFramesRemaining = 0;
		TimeDilation = 1;
		NetBruteForceCorrection = 0;
		return;
	}*/

	LockConfig();
	u32 framePadCfg = GetUserConfig()->Net.FramePad;
	UnlockConfig();

	if (NetDilationFramesRemaining > 0)
	{
		NetDilationFramesRemaining--;
		NetDilationFramesRemaining = KMin(NetDilationFramesRemaining, 1000);
	}

	if (NetBruteForceCorrection != 0 || (NetDilationFramesRemaining == 0 && TimeDilation != 1))
	{
		GetGameMatch()->ForceFrameCountAdjust(NetBruteForceCorrection);
		TimeDilation = 1;
		NetBruteForceCorrection = 0;
		LastNetStabilizeFrame = KTime::FrameCount();
	}

	f32 offset = player->ClientFrameDiff;
	f32 target = 1 + framePadCfg; // TODO detect LAN and make it .25
	target += std::clamp(player->TotalFrameDrops, (u8)0, (u8)10);

	f32 diff = offset - target;			// force us to wait to see result of adjustment before going again
	if (player->LastInputFrame >= LastNetStabilizeFrame && player->LastInputFrame > 0)
	{
		if (offset <= 0)
		{
			NetBruteForceCorrection = -offset + 1;
		}
		/*else if (offset > 60)
		{
			NetBruteForceCorrection = -offset;	
		}*/
		else if (abs(diff) > .5)
		{
			if (NetDilationFramesRemaining == 0)
			{
				f32 change = 0;
				change = MapRange(abs(diff), 7, 15, .05, .5);
				change = KClamp(change, .05f, .5f);

				TimeDilation = diff > 0 ? 1 - change : 1 + change;
				
				// at 5% speed offset, 20 frames = one frame off
				NetDilationFramesRemaining = (1.f / abs(1.f - TimeDilation)) * abs(diff);
				if (offset > 0) NetDilationFramesRemaining /= 2;
			}
		}
	}

	NetStats.TimeDilation = TimeDilation;
}

void KNetInterface_Client::ProcessGameState(const u8* data, u32 dataSize)
{
	//KSystemTerminal::Show();
	//_SYSLOG("-----------------------------------------------------------");
	KNetPlayer* player = GetLocalNetPlayer();

	u32 cursor = 0;

	auto readData = [&cursor, data](void* dest, u8 byteCount) mutable -> void
	{
		memcpy(dest, data + cursor, byteCount);
		cursor += byteCount;
	};

	// TODO do we need to do this in KSnapshot::Apply?
		//if (KTime::FrameCount() == obj->SnapshottableFrameCreated)
		//{
		//	// if this property should only updated on the first snapshot,
		//	// dont update client data, instead just offset cursor
		//	if (obj->GetClientData()->PropertyIsFirstOnly(index))
		//	{
		//		cursor += size;
		//		return;
		//	}
		//}

	TVector<KSnapshottable*> createdThisFrame;
	TVector<KSnapshottable*> updatedThisFrame;

	struct PendingDestroy
	{
		KSnapshottable* Object = nullptr;
		bool bCreatedNow = false;
	};
	TVector<PendingDestroy> pendingDestroys;

	u8 damageNumberCount = 0;
	readData(&damageNumberCount, 1);
	TVector<KReppedDamageNumber> pendingDamageNumbers; // cant find the entity yet
	for (u32 i = 0; i < damageNumberCount; i++)
	{
		KReppedDamageNumber damage;
		readData(&damage, 12);

		// not new
		if (damage.Frame <= player->LastFinishedUnpackedFrame) continue;

		KSnapshottable* snap = GetReplicatedObject(damage.NetID);
		if (snap)
		{
			if (KEntity* ent = dynamic_cast<KEntity*>(snap))
				KDamageNumber d(ent->GetPosition().AdjustZ(32).ToType<f32>(), damage.Damage, 45);		
		}
		else
		{
			pendingDamageNumbers.push_back(damage);
		}
	}
	u16 destroyCount = 0;
	readData(&destroyCount, 2);
	//_SYSLOG("destroy count " + KString(destroyCount));

	if (destroyCount > 0)
		pendingDestroys.reserve(destroyCount);

	// destroy objects first
	for (destroyCount; destroyCount > 0; destroyCount--)
	{
		u8 classID;
		u32 netID;
		u8 owner;
		readData(&classID, 1);
		readData(&netID, 4);
		readData(&owner, 1);
		u32 paramSize = KDestroySnapshot::GetSize(classID);
		const u8* destroyPtr = data + cursor - 6;

		// check if this object was recently destroyed
		bool recentlyDestroyed = GetNetState()->NetIdWasRecentlyDestroyed(netID);
		_SYSLOG("destroy: " + KSnapshot::NameFromClassID(classID) + " ID: " + KString(netID) + " (" +  KString(recentlyDestroyed) + ")" + KString(owner));

		// check if we have this object, create it otherwise
		KSnapshottable* obj = nullptr;
		bool createdNow = false;
		if (recentlyDestroyed)
		{
			// already created and destroyed, do nothing
			cursor += paramSize - 6;
			continue;
		}
		else if (HasReplicatedObject(netID))
		{
			obj = GetReplicatedObject(netID);
		}
		else
		{	
			// weve never seen this before
			obj = KSnapshot::CreateReplicatedObject(classID, netID);
			createdNow = true;
		}

		KDestroySnapshot::Apply(obj, (u8*)destroyPtr);
		cursor += paramSize - 6;

		if (classID == KSnapshot::GetPlayerClassID())
		{
			// destroy players now on the off chance that another player is waiting to take its spot
			// TODO OnReps will never be applied but players dont have any at the moment anyway and probably never will
			if (createdNow) obj->InitNetObject();
			obj->OnNetDestroy();
			obj->DestroyNetObject(false);
		}
		else
		{
			pendingDestroys.push_back( { obj, createdNow } );
		}
	}

	// clear recent destroys for frames we wont receive again
	MapRemoveTo_Inclusive(GetNetState()->RecentClientDestroys, GetLocalNetPlayer()->LastAckedFrame);

	u32 snapshotSize = 0;
	readData(&snapshotSize, 4);
	snapshotSize += cursor;
	//_SYSLOG("snapshots");
	while (cursor < snapshotSize)
	{
		// expect member count
		u8 memberCount = 0;
		readData(&memberCount, 1);

		u32 netID = 0;
		readData(&netID, 4);

		u8 classID = 0;

		// if netID has this flag, this contains a classID
		//if (netID & 0x80000000) 
		{
			readData(&classID, 1);
			//netID &= 0x7FFFFFFF; // remove class ID flag
		}

		bool exists = HasReplicatedObject(netID);
		bool recentDestroy = GetNetState()->NetIdWasRecentlyDestroyed(netID);

		//_SYSLOG("exists " + KString(exists) + "member count " + KString(memberCount) + " netID " + KString(netID) + KSnapshot::NameFromClassID(classID) + " recent " + KString(recentDestroy));

		if (recentDestroy)
		{
			// we still need to create this to read the rest of the snapshot

			// TODO this wont be needed anymore since we dont need the object to know the parameter sizes

			KSnapshot::CreateReplicatedObject(classID, netID);
		}
		else if (!exists) 
		{
			K_ASSERT(classID != 0, "no class ID given to spawn net object - NetID " 
				+ KString(netID) + " - LastStateFrameReceived " + KString(GetLocalNetPlayer()->LastStateFrameReceived)
				+ " - CurrentFrame " + KString(KTime::FrameCount()));
			createdThisFrame.push_back(KSnapshot::CreateReplicatedObject(classID, netID));
			exists = true;
		}

		KSnapshottable* obj = GetReplicatedObject(netID);

		K_ASSERT(obj, "null object from valid net id");

		for (u32 i = 0; i < memberCount; i++)
		{
			u8 propIndex = 0;
			readData(&propIndex, 1);
			cursor += KSnapshot::Apply(propIndex, obj, data + cursor);
		}

		if (memberCount > 0 && !recentDestroy) 
			updatedThisFrame.push_back(obj);
		else if (recentDestroy) 
			obj->DestroyNetObject(true);

		if (classID == KSnapshot::LookUpClassID(typeid(KNetPlayer)))
		{
			if (VectorContains(createdThisFrame, obj)) 
			{
				// if this is the first time receiving a player, unpack it before everything else
				// players are always packed before any other object and therefore iterated first in this loop
				// this lets us adjust our local frame count before spawning new entities with a "bad" creation frame

				//obj->UnpackSnapshot();
				obj->InitNetObject();
				obj->OnNetUpdate();

				VectorRemove(updatedThisFrame, obj);
				VectorRemove(createdThisFrame, obj);
			}
		}
	}
	
	for (PendingDestroy& d : pendingDestroys)
		if (d.bCreatedNow) d.Object->InitNetObject();
		
	// TODO last thing to do is store OnReps in net_format

	KSnapshot::ApplyOnReps(true);

	for (PendingDestroy& d : pendingDestroys)
	{
		d.Object->OnNetDestroy();
		d.Object->DestroyNetObject(false);
	}

	// new objects are initialized now that their relevant data is set
	for (KSnapshottable* obj : createdThisFrame)
		obj->InitNetObject();

	KSnapshot::ApplyOnReps(false);

	for (KSnapshottable* obj : updatedThisFrame)
		obj->OnNetUpdate();

	for (KReppedDamageNumber& damage : pendingDamageNumbers)
	{
		if (KSnapshottable* snap = GetReplicatedObject(damage.NetID))
		  if (KEntity* ent = dynamic_cast<KEntity*>(snap))
			KDamageNumber d(ent->GetPosition().AdjustZ(32).ToType<f32>(), damage.Damage, 45);
	}

	// read sounds
	u8 soundCount = 0;
	readData(&soundCount, 1);
	for (u8 i = 0; i < soundCount; i++)
	{
		// PreparePack() in replicated_sound.cpp has order

		u8 flags = 0;
		KSoundID id = 0;
		u32 frame = 0;

		readData(&flags, 1);
		readData(&id, sizeof(KSoundID));
		readData(&frame, 4);

		u8 volume = 255;
		u16 maxDist = 2048;
		GVec3 pos;
		u32 entID = 0;
		KEntity* attachEnt = nullptr;

		if (flags & KReppedSound::HasVolume)
			readData(&volume, 1);
		if (flags & KReppedSound::HasAttenuation)
			readData(&maxDist, 2);

		KSoundProperties props;
		props.bAttachedPlayAfterDeath = flags & KReppedSound::AttachedPlayAfterDeath;
		props.MaxDistance = maxDist;
		props.Volume = (f32)volume / 255.f;

		if (flags & KReppedSound::Position16)
		{
			i16 x = 0, y = 0, z = 0;
			readData(&x, 2);
			readData(&y, 2);
			readData(&z, 2);
			pos = GVec3(x, y, z);
		}
		else if (flags & KReppedSound::Position32)
		{
			f32 x = 0, y = 0, z = 0;
			readData(&x, 4);
			readData(&y, 4);
			readData(&z, 4);
			pos = GVec3(x, y, z);
		}
		else if (flags & KReppedSound::AtNetID)
		{
			readData(&entID, 4);
			if (KSnapshottable* snap = GetReplicatedObject(entID))
				pos = dynamic_cast<KEntity*>(snap)->GetPosition();
			else
				continue;
		}
		else if (flags & KReppedSound::AttachNetID)
		{
			readData(&entID, 4);
			if (KSnapshottable* snap = GetReplicatedObject(entID))
				attachEnt = dynamic_cast<KEntity*>(snap);
			else
				continue;
		}

		if (frame <= player->LastFinishedUnpackedFrame) continue;

		if (attachEnt)
			KAudio::PlaySoundAttached(id, attachEnt, props);
		else
			KAudio::PlaySound3D(id, pos, props);
	}

	// client allocates new player objects in CreateReplicatedObject
	// player's InitNetObject copies it to the net interface 
	// this function frees the temporary object
	//GetNetState()->ClearPendingPlayers();

	player->LastFinishedUnpackedFrame = player->LastStateFrameUnpacked;
	GetNetState()->bReceivingUpdates = true;
}

void KNetInterface_Client::NotifyServerLoadedMap()
{
	// send our name with it
	u8 nameSize = 0;
	UPtr<u8[]> nameData = GetLocalPlayerNameAsData(nameSize);

	KNetPacket p(nameData.get(), nameSize, EMessageType::C_LoadedMap);
	Server->SendPacket(&p);
}

void KNetInterface_Client::ProcessUpdates()
{
	KNetInterface::ProcessUpdates();

	// check if we should unpack a snapshot
	if (KNetState* state = GetNetState())
	{
		state->UnpackNextSnapshot();
		state->CurrentInterpFrame = state->GetCurrentInterpFrame();
	}
}

#if _DEV
void KNetInterface_Client::RequestServerQuit()
{
	u8 d = 0;
	KNetPacket p(&d, 1, EMessageType::C_ServerQuit);
	Server->SendPacket(&p);
}
#endif

class KNetPlayer* KNetInterface_Client::GetPlayerFromIndex(u8 index)
{
	if (ConnectedPlayers.contains(index))
		return ConnectedPlayers[index];

	return nullptr;
}

void KNetInterface_Client::OnReceive(class KNetPlayer* sender, const u8* data, u32 dataSize, EMessageType type, u32 frameNumber)
{
	// sender is null, but never needed because it can only be the server

	switch (type)
	{
		case EMessageType::S_Ping:
		{
			u8 id = *data;
			KNetPacket pong(&id, 1, EMessageType::C_Pong);
			Server->SendPacket(&pong);
			break;
		}
		case EMessageType::S_GameState:
		{
			if (GetGameMatch() && GetNetState() && !KGameInstance::Get().IsLoadingMap())
			{
				// first four bytes should always be client frame diff
				memcpy(&GetLocalNetPlayer()->ClientFrameDiff, data, 4);
				NetStats.ClientFrameDiff = GetLocalNetPlayer()->ClientFrameDiff;
				
				if (dataSize > 4)
					GetNetState()->BufferSnapshotData(data + 4, dataSize - 4, frameNumber);

				//ProcessGameState(data, dataSize);
				/*SnapshotBuffer.push_back(KBufferedSnapshot(data, dataSize, frameNumber));
				if (SnapshotBuffer.size() > 5)
				{
					KBufferedSnapshot& buff = SnapshotBuffer[SnapshotBuffer.size() - 1];
					ProcessGameState(buff.Data.get(), buff.Size);
					SnapshotBuffer.pop_back();
				}*/
			}
			break;
		}
		case EMessageType::S_Welcome:
		{
			if (KServerWelcome::ReceiveAndProcess(*(KServerWelcome*)(data)))
			{
				bIsWelcomed = true;
				NotifyServerLoadedMap();
				GetJoinMenu()->OpenMenu();
			}
			// else go get the map and try connecting again
			break;
		}
		case EMessageType::S_MapSwitch:
		{
			KGameInstance& inst = KGameInstance::Get();

			KMapHash hash;
			memcpy(&hash, data, 16);
			if (inst.HasMapWithHash(hash))
				SetPendingServerMap(inst.GetLocalMapByHash(hash).MapFileName);
				//inst.LoadMap(inst.GetLocalMapByHash(hash).MapFileName);
			// else else download map
			
			//NotifyServerLoadedMap();
			break;
		}
		case EMessageType::S_NeedPassword:
			break;
		case EMessageType::S_UserChat:
		case EMessageType::S_Message:
		case EMessageType::S_MessageWarning:
		case EMessageType::S_MessageError:
		case EMessageType::S_ConsoleMessage:
		case EMessageType::S_ConsoleWarning:
		case EMessageType::S_ConsoleError:
		{
			KString message = "";
			message.GetMut().resize(dataSize, '\0');
			memcpy((void*)message.Get().data(), data, dataSize);
			KGameInstance::Get().ReceiveChatMessage(message, u8(type));
			break;
		}
	}
}

void KNetInterface_Client::OnConnection(class KNetPeer* peer)
{
	KString ip = GetServerIP();
	GetLocalPlayer()->GetConfig()->SetLastConnectedIP(ip);
	
	LOG("Connected to " + ip);
}

void KNetInterface_Client::OnDisconnect(class KNetPeer* peer)
{
	
}

void KNetInterface_Client::SendFrame()
{
	KNetPlayer* player = GetLocalPlayer()->LocalNetPlayer;
	if (!player || !GetGameMatch() || !GetNetState() || !player->IsReceivingState()) return;

	u32 packetSize = 0;
	UPtr<u8[]> inputData;
	player->BuildInputPacket(inputData, packetSize);

	if (inputData.get())
	{
		KNetPacket packet(inputData.get(), packetSize, EMessageType::C_Input);
		Server->SendPacket(&packet);
	}

	// send name update if we have it
	if (bPendingNameChange)
	{
		u8 nameSize = 0;
		UPtr<u8[]> nameData = GetLocalPlayerNameAsData(nameSize);
		if (nameSize > 0)
		{
			KNetPacket packet(nameData.get(), nameSize, EMessageType::C_NameUpdate);
			Server->SendPacket(&packet);
		}
		bPendingNameChange = false;
	}

	enet_host_flush(Host->Host);
}

void KNetInterface_Client::SendChatMessage(const KString& msg, KNetPeer* peer, EMessageType type)
{
	KNetPacket packet((u8*)msg.CStr(), msg.Size(), EMessageType::C_ChatMessage);
	Server->SendPacket(&packet);
}

void KNetInterface_Client::PackPlayerInfo(TVector<KPlayerScoreInfo>& info)
{
	info.clear();
	for (const auto& kv : ConnectedPlayers)
	{
		KNetPlayer* guy = kv.second;
		AddPlayerToScoreInfo(guy, info);
	}
}

void KNetInterface_Client::ClearConnectedPlayers()
{
	for (const auto& kv : ConnectedPlayers)
		delete kv.second;
	
	ConnectedPlayers.clear();
}

class KSnapshottable* KNetInterface_Client::GetReplicatedObject(u32 netID)
{
	if (netID >= 0xFFFFFF00)
	{
		u32 index = netID & 0x000000FF;
		return GetPlayerFromIndex(index);
	}

	if (GetNetState())
		return GetNetState()->GetReplicatedObject(netID);

	return nullptr;
}

bool KNetInterface_Client::HasReplicatedObject(u32 netID)
{
	if (netID >= 0xFFFFFF00)
	{
		u32 index = netID & 0x000000FF;
		return GetPlayerFromIndex(index);
	}

	if (GetNetState())
		return GetNetState()->HasReplicatedObject(netID);

	return false;
}

void KNetInterface_Client::SetPendingServerMap(const KString& map)
{
	PendingServerMap = map;
}

void KNetInterface_Client::FollowServerMap()
{
	if (!PendingServerMap.IsEmpty())
	{
		KGameInstance::Get().LoadMap(PendingServerMap);

		PendingServerMap = "";
		if (KNetPlayer* p = GetLocalNetPlayer())
			p->NewMapReset();

		NotifyServerLoadedMap();
	}
}

void KNetInterface_Client::CallServerMap(const KString& map)
{
	KNetPacket packet((u8*)map.CStr(), map.Size(), EMessageType::C_ServerMap);
	Server->SendPacket(&packet);
}

#endif