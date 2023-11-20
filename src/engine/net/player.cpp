#include "player.h"
#include "data/peer.h"
#include "engine/game/local_player.h"
#include "data/packet.h"
#include "state.h"
#include "net_interface.h"
#include "server.h"
#include "engine/game/match.h"
#include "../math/packed_float.h"
#include "../input/listener_game.h"
#include "replicated_sound.h"
#include "../../game/entity/spectator.h"
#include "client.h"
#include "game_state.h"

KString CCOM_Spectate(const KString& val)
{
	if (KNetPlayer* p = GetLocalNetPlayer())
	{
		p->ViewedPlayerIndex = NULL_PLAYER;
	}

	return "";
}

KString CCOM_Join(const KString& val)
{
	if (KNetPlayer* p = GetLocalNetPlayer())
	{
		if (p->ViewedPlayerIndex != p->OwningPlayerIndex)
		{
			GetLocalPlayer()->bPendingSpawnRequest = true;
			p->ViewedPlayerIndex = p->OwningPlayerIndex;
		}			
	}
	return "";
}

void KNetPlayer::GetTransient_ReppedName(KReppedPlayerName& val)
{
	val.NameSize = PlayerName.Size();
	memcpy(val.Name, PlayerName.CStr(), PlayerName.Size());
}

void KNetPlayer::SetTransient_ReppedName(KReppedPlayerName& val)
{
	PlayerName.GetMut().resize(val.NameSize, '\0');
	memcpy((void*)PlayerName.Get().data(), val.Name, val.NameSize);
}

void KNetPlayer::GetTransient_PackedPing(u8& val)
{
	val = PlayerPing.GetDividedPing();
}

void KNetPlayer::SetTransient_PackedPing(u8& val)
{
	ClientPing = val * 4;	
}

void KNetPlayer::GetTransient_bUpdateScoreboard(bool& val)
{
	val = IsNetServer() ? dynamic_cast<KNetInterface_Server*>(GetNetInterface())->bUpdateClientScoreboards : true;
}

void KNetPlayer::SetTransient_bUpdateScoreboard(bool& val)
{
	if (!GetNetInterface()->bPendingScoreUpdate)
		GetNetInterface()->bPendingScoreUpdate = val;
}

bool KNetPlayer::bMatchTick = false;

KNetPlayer* GetLocalNetPlayer()
{
	return GetLocalPlayer()->LocalNetPlayer;
}

void KNetPlayer::InitNetObject()
{
	GetNetInterface()->AddPlayerToMap(this);
}

void KNetPlayer::BufferSnapshotData(u8* data, u32 byteCount)
{
	StateSendBuffer.insert(StateSendBuffer.end(), data, data + byteCount);
}

void KNetPlayer::PopSnapshotData(u32 bytesToRemove)
{
	StateSendBuffer.resize(StateSendBuffer.size() - bytesToRemove);
}

bool KNetPlayer::IsLocalPlayer() const
{
	return OwningPlayerIndex == GetLocalPlayer()->NetPlayerIndex;
}

bool KNetPlayer::IsViewedPlayer() const
{
	return IsLocalPlayer();
}

void KNetPlayer::BuildInputPacket(UPtr<u8[]>& inputBuffer, u32& packedSize)
{
	ClearOldStoredInput();
	AddCurrentInputFrame();

	KNetState* state = GetNetState();
	if (!state) return;

	u32 lastAcked = LastStateFrameReceived;
	u8 lastUnpackedDiff = (u8)std::clamp(lastAcked - state->CurrentInterpFrame, 0u, 255u);
	u32 current = KTime::FrameCount();

	// store the time we ran this frame so we can measure our real delay when the server sends state for it
	TimeOfFrames[current] = KTime::Now();

	TVector<KPackedClientInput> pendingPack;
	packedSize = 9; // last acked, last unpacked diff, and current

	const KClientInputFrame* lastFrame = nullptr;
	u32 inputCount = 0;
	for (const KClientInputFrame& input : StoredLocalInput)
	{
		KPackedClientInput pack;
		pack.SetInputData(input);
		if (!lastFrame)
		{
			pack.AddAll();

			// add entire frame first
			packedSize += 13;
		}
		else
		{
			u32 preSize = packedSize;

			if (lastFrame->Flags != input.Flags);
			{
				pack.AddFlags();
				packedSize += 1;
			}

			if (lastFrame->MovementKeyFlags != input.MovementKeyFlags)
			{
				pack.AddKeys();
				packedSize += 1;
			}

			if (lastFrame->WeaponIndex != input.WeaponIndex)
			{
				pack.AddWeapon();
				packedSize += 1;
			}

			if (lastFrame->Pitch != input.Pitch)
			{
				pack.AddPitch();
				packedSize += 2;
			}
			
			if (lastFrame->Yaw != input.Yaw)
			{
				pack.AddYaw();
				packedSize += 2;
			}

			if (lastFrame->ViewedPlayer != input.ViewedPlayer)
			{
				pack.AddView();
				packedSize += 1;
			}

			if (preSize != packedSize)
			{
				// add frame and flags
				packedSize += 5;
			}
		}

		pendingPack.push_back(pack);
		lastFrame = &input;
		inputCount++;
		if (inputCount >= 25) break;
	}

	if (packedSize == 0) return;

	inputBuffer = std::make_unique<u8[]>(packedSize);
	u32 cursor = 0;
	
	const auto copyData = [&inputBuffer, &cursor, packedSize](const void* data, u8 size) -> void
	{
		memcpy(inputBuffer.get() + cursor, data, size);
		cursor += size;
		K_ASSERT(cursor <= packedSize, "client input buffer overrun");
	};

	copyData(&lastAcked, 4);
	copyData(&lastUnpackedDiff, 1);
	copyData(&current, 4);

	for (const KPackedClientInput& p : pendingPack)
	{
		if (p.HasNoFlags()) continue;
		const KClientInputFrame& inputData = p.GetInputData();

		copyData(&inputData.Frame, 4);

		u8 flags = p.GetFlags();
		copyData(&flags, 1);
		
		if (p.HasFlags())   copyData(&inputData.Flags, 1);
		if (p.HasKeys())    copyData(&inputData.MovementKeyFlags, 1);
		if (p.HasWeapon())  copyData(&inputData.WeaponIndex, 1);
		if (p.HasPitch())   copyData(&inputData.Pitch, 2);
		if (p.HasYaw())     copyData(&inputData.Yaw, 2);
		if (p.HasView())    copyData(&inputData.ViewedPlayer, 1);
	}
}

void KNetPlayer::ClearOldStoredInput()
{
	u32 frame = KTime::FrameCount();
	if (frame < (ClientFrameDiff + 1)) return;

	u32 index = 0;
	u32 cutoff = frame - ClientFrameDiff - 1;

	for (index; index < StoredLocalInput.size(); index++)
	  //if (StoredLocalInput[index].Frame >= LastStateFrame)
	  if (StoredLocalInput[index].Frame >= cutoff)
		break;

	if (index > 0)
		StoredLocalInput.erase(StoredLocalInput.begin(), StoredLocalInput.begin() + index);
}

KNetPlayer::KNetPlayer()
{
	ClassID = 223;
}

/*
void KNetPlayer::StoreInput(KClientInputFrame frame)
{
	LocalClientInput.push_back(std::move(frame));
}*/

KNetPlayer::~KNetPlayer() 
{
	if (GetLocalPlayer()->LocalNetPlayer == this)
		GetLocalPlayer()->LocalNetPlayer = nullptr;

	if (KEntity* ent = ControlledEntity.Get())
		ent->DestroyEntity();

	if (NetPeer)
	{
		NetPeer->DisconnectPeer(0);
		delete NetPeer;
	}
}

void KNetPlayer::OnNetUpdate()
{
	bReceivingGameState = true;
}

void KNetPlayer::Tick()
{
	if (IsNetServer())
	{
		if (bMatchTick)
		{
			if (bPendingSpawnRequest && FinishLoadingFrame != 0)
			{
				if (!ControlledEntity.IsValid())
					ControlledEntity = GetGameMatch()->SpawnPlayerCharacter(OwningPlayerIndex);
				bPendingSpawnRequest = false;
			}
		}
		else if (!IsLocalPlayer())
		{
			if (!IsReceivingState()) return;
			SendPingRequest();
			SendGameState();
			RemoveOldData();
		}
	}

	if (IsSpectating())
	{
		if (IsNetServer())
		{
			if (!bSpectatingLastFrame)
			{
				// announce the switch
				GetNetInterface()->SendChatMessage(PlayerName + " joined the spectators", nullptr, EMessageType::S_Message);
			}

			bSpectatingLastFrame = true;

			if (KEntity* ent = ControlledEntity.Get())
			{
				if (!ent->As<KEntity_Spectator>())
					ent->DestroyEntity();
			}
		}

		if (IsLocalPlayer())
		{
			// wont actually spawn new spectator if we already have one
			GetLocalPlayer()->PossessSpectator();
		}
	}
	else if (IsNetServer())
	{
		if (bSpectatingLastFrame)
			GetNetInterface()->SendChatMessage(PlayerName + " entered the arena", nullptr, EMessageType::S_Message);

		bSpectatingLastFrame = false;
	}
}

KBufferedClientInput* KNetPlayer::GetMostRecentInput(u32& frameNumber)
{
	if (ClientInput.size() > 0)
	{
		auto entry = ClientInput.rbegin();
		frameNumber = entry->first;
		return &(entry->second);
	}

	return nullptr;
}

void KNetPlayer::AddClientInput(const KClientInputFrame& frame)
{
	//KClientInputFrame::BufferedInput newinput;
	//newinput.Keys = frame.MovementKeyFlags;
	//newinput.WeaponIndex = frame.WeaponIndex;
	//newinput.Pitch = frame.Pitch;
	//newinput.Yaw = frame.Yaw;
	//ClientInput[frame.CurrentNetFrame] = newinput;
}

void KNetPlayer::NewMapReset()
{
	FinishLoadingFrame = 0;
	LastStateFrameReceived = 0;
	LastStateFrameUnpacked = 0;
	LastFinishedUnpackedFrame = 0;
	ClientFrameDiff = 0;
	LastAckedFrame = 0;
	TotalFrameDrops = 0;
	LastInputFrame = 0;
	bReceivingGameState = false;
	LostInputFrames.clear();
	StoredLocalInput.clear();
	TimeOfFrames.clear();
	ControlledEntity = nullptr;
	ClientInput.clear();
	bPendingSpawnRequest = false;

#if !_SERVER
	if (IsLocalPlayer())
	{
		if (KNetInterface_Client* c = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		{
			c->TimeDilation = 1;
			c->NetDilationFramesRemaining = 0;
			c->LastNetStabilizeFrame = 0;
			c->NetBruteForceCorrection = 0;
		}
	}
#endif
}

void KNetPlayer::AddCurrentInputFrame()
{
#if !_SERVER
	KClientInputFrame frame;
	
	bool has = dynamic_cast<KSnapshottable*>(GetControlledEntity());
	frame.Frame = KTime::FrameCount();
	frame.MovementKeyFlags = has ? GetGameInput()->GetState() : 0;
	frame.Pitch = has ? GetGameInput()->GetPitch() : 0;
	frame.Yaw = has ? GetGameInput()->GetYaw() : 0;

	if (GetLocalPlayer()->HasPendingSpawnRequest())
	{
		frame.IncludeSpawnRequest();
		ViewedPlayerIndex = OwningPlayerIndex;
	}

	frame.ViewedPlayer = ViewedPlayerIndex;

	if (KEntity* ent = GetControlledEntity())
	  if (KEntProp_WeaponInventory* inv = ent->As<KEntProp_WeaponInventory>())
	    frame.WeaponIndex = inv->GetPendingWeaponIndex() + (u8)EInputAction::Weapon0;

	StoredLocalInput.push_back(frame);
#endif
}

void KNetPlayer::SendPingRequest()
{
	// send ping request 
	if (KTime::FrameCount() % 60 == 0)
	{
		KNetPacket ping(&PingRequestID, 1, EMessageType::S_Ping);
		NetPeer->SendPacket(&ping);
		PendingPingRequests[PingRequestID] = KTime::Now();
		PingRequestID++;
	}
}

void KNetPlayer::RemoveOldData()
{
	u32 oldestAcceptableFrame = KTime::FrameCount() > 30 ? KTime::FrameCount() - 30 : 0;
	MapRemoveTo_Exclusive(ClientInput, oldestAcceptableFrame);

	// remove dropped packets from > 5 seconds ago
	u32 maxframes = 60 * 5;
	u32 cutoff = KTime::FrameCount() < maxframes ? 0 : KTime::FrameCount() - maxframes;
	i32 index = 0;
	for (index; index < LostInputFrames.size(); index++)
	  if (LostInputFrames[index] > cutoff)
		break;

	if (index > 0) 
		LostInputFrames.erase(LostInputFrames.begin(), LostInputFrames.begin() + index);
}

void KNetPlayer::SendGameState()
{
	TotalFrameDrops = (u8)std::clamp((u32)LostInputFrames.size(), 0u, 255u) / 2;

	// track the oldest frame we need to hold data for
	//if (LastStateFrame != 0) <-- removed
	// we should keep all destroys if no frames have been acked yet
	// ack could be on the way and then this client would have no way of deleting
	// anything that went with whatever frame theyre receiving
	if (KNetInterface_Server* server = dynamic_cast<KNetInterface_Server*>(GetNetInterface()))
		server->LowestAckedFrame = KMin<u32>(server->LowestAckedFrame, LastAckedFrame);

	// remove old damage numbers
	{
		i32 index = -1;
		for (u32 i = 0; i < OutDamageNumbers.size(); i++)
			if (OutDamageNumbers[i].Frame < LastAckedFrame)
				index = i;

		if (index >= 0)
			OutDamageNumbers.erase(OutDamageNumbers.begin(),
				OutDamageNumbers.begin() + index + 1);
	}

	// give frame diff immediately so we dont need to unpack the snapshot to get it
	// this prevents more lag from being added with snapshot buffering
	{
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		memcpy(StateSendBuffer.data(), &ClientFrameDiff, 4);
	}

	if (KTime::FrameCount() % SNAPSHOT_INTERVAL == 0)
	{
		KGameStateSnapshot snapshot(LastAckedFrame, &StateSendBuffer, this);
#if 0
		// send damage numbers
		// client will try to apply them as long as the net ID exists
		// will try again after creating new entities if any remain
		u8 damageCount = (u8)(std::clamp(OutDamageNumbers.size(), size_t(0), size_t(255)));
		BufferSnapshotData(&damageCount, 1);
		for (u32 i = 0; i < OutDamageNumbers.size() && i < 255; i++)
		{
			const KReppedDamageNumber& damage = OutDamageNumbers[i];
			BufferSnapshotData((u8*)&damage, 12);
		}

		// add destroys
		// create 2 slots for 16 bit count
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		size_t countSlot = StateSendBuffer.size() - 2;
		u16 destroyCount = 0;
		for (const auto& kv : GetNetState()->DestroyedNetItems)
		{	
			// comparing destroy frame to last acked frame
			// its possible theyre equal and the client hasnt received this yet
			// disconnections happen between frames, before frame count is incremented
			// TODO cleaner solution is to check (kv.first <= LastStateFrame) and
			// mark disconnects as pending for next frame rather than handling immediately
			if (kv.first < LastAckedFrame && LastAckedFrame > 0) continue;
			for (const KNetState::DestroyNetData& obj : kv.second)
			{
				BufferSnapshotData( (u8*)obj.Object.get(), obj.Size );
				destroyCount++;
			}
		}
		memcpy(StateSendBuffer.data() + countSlot, &destroyCount, 2);

		// important to remember that because objects are destroyed during the frame
		// and snapshots are packed after the frame, snapshots are not created for objects 
		// on the frame they are destroyed and we can only remember them through destroy data

		// add snapshots
		// create 4 slots for 32 bit count
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		StateSendBuffer.push_back(0);
		size_t snapSlot = StateSendBuffer.size() - 4;
		u32 snapSize = StateSendBuffer.size();

		TDataPool<KNetPlayer>::GetPool()->PackSnapshotsForPlayer(LastAckedFrame, OwningPlayerIndex, &StateSendBuffer);
		for (const auto& kv : *KDataPoolBase::GetSnapshotPoolMap())
			kv.second->PackSnapshotsForPlayer(LastAckedFrame, OwningPlayerIndex, &StateSendBuffer);

		snapSize = StateSendBuffer.size() - snapSize;
		memcpy(StateSendBuffer.data() + snapSlot, &snapSize, 4);

		// add sounds
		StateSendBuffer.push_back(0); // count slot
		countSlot = StateSendBuffer.size() - 1;
		u8 soundCount = 0;
		for (const auto& kv : GetNetState()->PendingNetSounds)
		{
			if (kv.first < LastAckedFrame && LastAckedFrame > 0) continue;
			for (const KReppedSound& sound : kv.second)
			{
				// sounds have a cutoff frame if they were created with bLowHoldFrames = true
				// prevents clients from getting bombarded with useless sounds after losing a lot of frames
				if (KTime::FrameCount() > sound.CutoffFrame) continue;

				// make sure this player needs this sound
				if (sound.RepMethod == EReplicatedSound::SendIndex)
				{
					if (OwningPlayerIndex != sound.PlayerIndex) continue;
				}
				else if (sound.RepMethod == EReplicatedSound::SkipIndex)
				{
					if (OwningPlayerIndex == sound.PlayerIndex) continue;
				}

				BufferSnapshotData((u8*)sound.PackedData, sound.AllocSize);
				soundCount++;

				if (soundCount == 255) break; // 255 wtf
			}
			if (soundCount == 255) break; // double break instead of badass goto statement
		}
		memcpy(StateSendBuffer.data() + countSlot, &soundCount, 1);
#endif
	}

	// send data
	u32 size = StateSendBuffer.size();
	KNetPacket statePacket(StateSendBuffer.data(), size, EMessageType::S_GameState);
	NetPeer->SendPacket(&statePacket);

	// data ideally is still allocated after clear so reserve does nothing
	StateSendBuffer.clear();
	StateSendBuffer.reserve(size); 
}
