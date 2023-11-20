#include "server.h"
#include "player.h"
#include "data/peer.h"
#include "data/host.h"
#include "messages/client_input.h"
#include "state.h"
#include "engine/game_instance.h"
#include "engine/game/match.h"
#include "data/packet.h"
#include "data/address.h"
#include "game/cheat_manager.h"
#include "game_state.h"

KNetInterface_Server::KNetInterface_Server(u16 port)
{
	Config.LoadConfig();
	Config.Port = port;

	InitEnet();
	InterfaceType = ENetInterfaceType::Server;
#if _SERVER
	InterfaceType |= ENetInterfaceType::Dedicated;
#endif

	bIsWelcomed = true;

	if (TDataPool<KNetPlayer>* pool = TDataPool<KNetPlayer>::GetPool())
	{
		pool->DestroyAll();
	}
	else
	{
		// dont add player pool to net pools
		// needs to pack first and thus manually
		TDataPool<KNetPlayer>::CreatePool(255, 0, KPool::EOverflowResponse::PreventNew);
	}

#if !_SERVER
	// this is a player hosting
	KNetPlayer* newPlayer = CreateNewPlayer();
	LockConfig();
	newPlayer->PlayerName = GetUserConfig()->Net.PlayerName;
	UnlockConfig();
	GetLocalPlayer()->NetPlayerIndex = newPlayer->OwningPlayerIndex;
	GetLocalPlayer()->LocalNetPlayer = newPlayer;
	newPlayer->ViewedPlayerIndex = newPlayer->OwningPlayerIndex;
	newPlayer->ControlledEntity = GetControlledEntity();
#endif

	KCheatManager::ClearPersistentCheats();

	Host = std::make_unique<KNetHost>(PEER_COUNT, port);

	if (GetGameMatch()) GetGameMatch()->InitNet();

	LOG("Hosting on port " + KString(Host->Host->address.port));
}

KNetInterface_Server::~KNetInterface_Server()
{
	if (TDataPool<KNetPlayer>* pool = TDataPool<KNetPlayer>::GetPool())
		pool->DestroyAll();
}

KNetPlayer* KNetInterface_Server::CreateNewPlayer()
{
	TObjRef<KNetPlayer> player = TDataPool<KNetPlayer>::GetPool()->CreateNew();
	if (KNetPlayer* p = player.Get())
	{
		p->OwningPlayerIndex = player.GetPoolIndex();
		p->PlayerName = "Player_" + KString(p->OwningPlayerIndex);
		if (GetNetInterface()) GetNetInterface()->FlagPendingScoreUpdate();
		return p;
	}
	return nullptr;
}

KNetPlayer* KNetInterface_Server::GetPlayerFromIndex(u8 index)
{
	TDataPool<KNetPlayer>* pool = TDataPool<KNetPlayer>::GetPool();
	if (pool->IndexIsActive(index))
		return (KNetPlayer*)pool->GetObjectAtIndex(index);

	return nullptr;
}

void KNetInterface_Server::OnReceive(KNetPlayer* sender, const u8* data, u32 dataSize, EMessageType type, u32 frameNumber)
{
	switch (type)
	{
		case EMessageType::C_Pong:
		{
			u8 id = *data;
			if (sender->PendingPingRequests.contains(id))
			{
				sender->PlayerPing.AddPingEntry(KTime::Since(sender->PendingPingRequests[id]) * 1000);
				sender->ClientPing = sender->PlayerPing.GetPing();
				sender->PendingPingRequests.erase(id);
			}
			break;
		}
		case EMessageType::C_Input:
		{
			if (!GetGameMatch() || KTime::FrameCount() == 0) return;

			u32 cursor = 0;
			auto readData = [&cursor, data, dataSize](void* dest, u8 byteCount) mutable -> void
			{
				memcpy(dest, data + cursor, byteCount);
				cursor += byteCount;
				K_ASSERT(cursor <= dataSize, "read over client input buffer length");
			};

			if (sender->FinishLoadingFrame == 0)
			{
				sender->FinishLoadingFrame = KTime::FrameCount();

				// TODO dont automatically spawn them, let player choose when to join
				//sender->bPendingSpawnRequest = true;
			}

			readData(&sender->LastAckedFrame, 4);

			u8 clientSnapshotFrameOffset;
			readData(&clientSnapshotFrameOffset, 1);

			u32 currentFrame;
			readData(&currentFrame, 4);
			if (sender->LastInputFrame < currentFrame)
			{
				sender->LastInputFrame = currentFrame;

				GFlt receiveFrame = KTime::FrameCount() + GetFrameAlpha();
				sender->ClientFrameDiff = (f32)sender->LastInputFrame - receiveFrame;
				//if (abs(sender->ClientFrameDiff) > 50) sender->BruteForceCorrection = -sender->ClientFrameDiff;
				//else sender->BruteForceCorrection = 0;
			}
			else
			{
				break;
			}

			u32 lastReadFrame = 0;
			u32 inputFrame = 0;
			bool addedInput = false;
			KBufferedClientInput input;
			input.ClientCompFrame = sender->LastAckedFrame - clientSnapshotFrameOffset;

			while (cursor < dataSize)
			{
				// first frame should have everything set

				readData(&inputFrame, 4);

				if (lastReadFrame != 0) 
				{
					// if reading a new frame, set all frames from lastReadFrame to frame-1 with input
					addedInput = true;
					for (u32 i = lastReadFrame; i < inputFrame; i++)
						sender->ClientInput[i] = input;
				}

				lastReadFrame = inputFrame;

				KPackedClientInput packed;
				readData(&packed.Flags, 1);

				u8 flags = 0;

				if (packed.HasFlags())  readData(&flags, 1);
				if (packed.HasKeys())   readData(&input.Keys, 1);
				if (packed.HasWeapon()) readData(&input.WeaponIndex, 1);
				if (packed.HasPitch())  readData(&input.Pitch, 2);
				if (packed.HasYaw())    readData(&input.Yaw, 2);
				if (packed.HasView())   readData(&sender->ViewedPlayerIndex, 1);
				
				if (flags & KClientInputFrame::CPF_RequestingSpawn) sender->bPendingSpawnRequest = true;
			}

			//if (lastReadFrame != inputFrame || !addedInput)
			  for (u32 i = lastReadFrame; i <= currentFrame; i++)
				sender->ClientInput[i] = input;

			break;
		}
		case EMessageType::C_LoadedMap:
		{
			if (sender->IsReceivingState()) 
				break;

			sender->NewMapReset();

			// we can start sending state
			sender->SetReceivingState(true);

			// wait for player to acknowledge they've loaded the gamestate before spawning them

			ParsePlayerName(sender, data);

			SendChatMessage(sender->PlayerName + " connected", nullptr, EMessageType::S_Message);
			LOG(sender->PlayerName + " connected from " + sender->NetPeer->GetAddress().GetString());
			break;
		}
		case EMessageType::C_ChatMessage:
		{
			KString message;
			message.GetMut().resize(dataSize, '\0');
			memcpy((void*)message.Get().data(), data, dataSize);
			SendChatMessage(message, sender->NetPeer);
			break;
		}
		case EMessageType::C_NameUpdate:
		{
			KString preName = sender->PlayerName;
			ParsePlayerName(sender, data);
			if (preName.Get() != sender->PlayerName.Get())
			{
				SendChatMessage(preName + " renamed to " + sender->PlayerName, nullptr, EMessageType::S_Message);
				LOG(preName + " renamed to " + sender->PlayerName);
				GetNetInterface()->FlagPendingScoreUpdate();
			}
		
			break;
		}
		case EMessageType::C_ServerMap:
		{
			KString map;
			map.GetMut().resize(dataSize, '\0');
			memcpy((void*)map.Get().data(), data, dataSize);
			KGameInstance::Get().LoadMap(map);
			break;
		}
#if _DEV
		case EMessageType::C_ServerQuit:
		{
			KGameInstance::Get().ExitGame();
			break;
		}
#endif
	}
}

void KNetInterface_Server::OnConnection(KNetPeer* peer)
{
	// player was already created
	LOG("Connection request from " + peer->GetAddress().GetString());
	peer->SendServerWelcome();
}

void KNetInterface_Server::OnDisconnect(KNetPeer* peer)
{
	KNetPlayer* p = peer->GetPlayer();
	SendChatMessage(p->PlayerName + " disconnected", nullptr, EMessageType::S_Message);
	LOG(p->PlayerName + " disconnected");
	if (p) p->Poolable_Destroy();
}

void KNetInterface_Server::SendFrame()
{
	LowestAckedFrame = MAX_U32;

#if !_SERVER
	if (bPendingNameChange)
	{
		if (KNetPlayer* p = GetLocalNetPlayer())
		{
			KString preName = p->PlayerName;
			LockConfig();
			KString newName = GetUserConfig()->Net.PlayerName;
			UnlockConfig();
			p->PlayerName = newName;

			if (preName.Get() != newName.Get())
			{
				SendChatMessage(preName + " renamed to " + p->PlayerName, nullptr, EMessageType::S_Message);
				LOG(preName + " renamed to " + p->PlayerName);				
			}
		}
		bPendingNameChange = false;
	}
#endif

	// create snapshots
	for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
		dynamic_cast<KNetPlayer*>(p)->CreateSnapshot();

	for (auto pool : SnapshotDataPools)
	  for (KPoolable* p : *pool)
	    dynamic_cast<KSnapshottable*>(p)->CreateSnapshot();

	// send snapshots and remove old input
	for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
		dynamic_cast<KNetPlayer*>(p)->Tick();

	enet_host_flush(Host->Host);

	if (KReplayWriter* r = KReplayWriter::GetActive())
	  if (r->IsWriting())
	    KGameStateSnapshot snapshot(KTime::FrameCount() - 1, r->GetBufferPointer(), GetLocalNetPlayer(), true);

	if (KTime::FrameCount() % SNAPSHOT_INTERVAL == 0)
		bUpdateClientScoreboards = false;

	// stop tracking destroyed objects and sounds that every client knows about
#if _SERVER
	if (TDataPool<KNetPlayer>::GetPool()->GetCurrentCount() == 0)
#else
	if (TDataPool<KNetPlayer>::GetPool()->GetCurrentCount() == 1) // local player
#endif
	{
		GetNetState()->DestroyedNetItems.clear();
		GetNetState()->PendingNetSounds.clear();
	}
	else if (LowestAckedFrame < MAX_U32 && LowestAckedFrame > 0)
	{
		if (GetNetState()->DestroyedNetItems.size() > 0)
			MapRemoveTo_Inclusive(GetNetState()->DestroyedNetItems, LowestAckedFrame); 

		if (GetNetState()->PendingNetSounds.size() > 0)
			MapRemoveTo_Inclusive(GetNetState()->PendingNetSounds, LowestAckedFrame); 
	}

	LowestAckedFrame = MAX_U32;
}

void KNetInterface_Server::SendChatMessage(const KString& msg, KNetPeer* peer /*= nullptr*/, EMessageType type /*= EMessageType::S_UserChat*/)
{
	KString message = peer ? (peer->GetPlayer()->PlayerName + ": " + msg) : msg;

	u32 size = message.Size();
	KNetPacket packet((u8*)message.CStr(), size, type);

	for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
	{
		KNetPlayer* player = dynamic_cast<KNetPlayer*>(p);
		if (player->NetPeer)
			player->NetPeer->SendPacket(&packet);
	}

#if !_SERVER
	KGameInstance::Get().ReceiveChatMessage(message, u8(type));
#endif
}

void KNetInterface_Server::PackPlayerInfo(TVector<KPlayerScoreInfo>& info)
{
	info.clear();
	for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
	{
		KNetPlayer* player = dynamic_cast<KNetPlayer*>(p);
		AddPlayerToScoreInfo(player, info);
	}
}

void KNetInterface_Server::NotifyClientsMapChange(u8 hash[16])
{
	for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
	{
		KNetPlayer* player = dynamic_cast<KNetPlayer*>(p);
		if (!player->NetPeer) return;
		player->NewMapReset();
		KNetPacket packet(hash, 16, EMessageType::S_MapSwitch);
		player->NetPeer->SendPacket(&packet);
	}
}

void KNetInterface_Server::ParsePlayerName(class KNetPlayer* player, const u8* data)
{
	u8 nameSize = 0;
	memcpy(&nameSize, data, 1);
	if (nameSize > 0)
	{
		player->PlayerName.GetMut().resize(nameSize, '\0');
		memcpy((void*)player->PlayerName.Get().data(), data + 1, nameSize > 32 ? 32 : nameSize);
		player->PlayerName.TrimInPlace();
	}
}

void KNetInterface_Server::FlagPendingScoreUpdate()
{
#if !_SERVER
	bPendingScoreUpdate = true;	// for our renderer
#endif
	bUpdateClientScoreboards = true; // for clients
}
