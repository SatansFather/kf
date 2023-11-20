#include "game_state.h"
#include "player.h"
#include "state.h"
#include "client.h"

KGameStateSnapshot::KGameStateSnapshot(u32 compareFrame, TVector<u8>* buffer, KNetPlayer* player, bool replay)
{
	PackForFrame(compareFrame, buffer, player, replay);
}

void KGameStateSnapshot::PackForFrame(u32 compareFrame, TVector<u8>* buffer, KNetPlayer* player, bool replay)
{
	Buffer = buffer;
	Player = player;
	CompareFrame = compareFrame;
	bStoringReplay = replay;

	u8 playerIndex = Player->OwningPlayerIndex;

	u32 sizeSlot = 0;
	if (replay)
	{
		Buffer->push_back(0);
		Buffer->push_back(0);
		Buffer->push_back(0);
		Buffer->push_back(0);
		sizeSlot = Buffer->size() - 4;
		//Player = nullptr;
		//playerIndex = NULL_PLAYER;
	}

	u32 dataSize = buffer->size();

	{
		// send damage numbers
		// client will try to apply them as long as the net ID exists
		// will try again after creating new entities if any remain
		if (Player)
		{
			u8 damageCount = (u8)(std::clamp(Player->OutDamageNumbers.size(), size_t(0), size_t(255)));
			BufferSnapshotData(&damageCount, 1);
			for (u32 i = 0; i < Player->OutDamageNumbers.size() && i < 255; i++)
			{
				const KReppedDamageNumber& damage = Player->OutDamageNumbers[i];
				BufferSnapshotData((u8*)&damage, 12);
			}
		}

		// add destroys
		// create 2 slots for 16 bit count
		Buffer->push_back(0);
		Buffer->push_back(0);
		size_t countSlot = Buffer->size() - 2;
		u16 destroyCount = 0;
		for (const auto& kv : GetNetState()->DestroyedNetItems)
		{
			// comparing destroy frame to last acked frame
			// its possible theyre equal and the client hasnt received this yet
			// disconnections happen between frames, before frame count is incremented
			// TODO cleaner solution is to check (kv.first <= LastStateFrame) and
			// mark disconnects as pending for next frame rather than handling immediately
			if (kv.first < CompareFrame && CompareFrame > 0) continue;
			for (const KNetState::DestroyNetData& obj : kv.second)
			{
				BufferSnapshotData((u8*)obj.Object.get(), obj.Size);
				destroyCount++;
			}
		}
		memcpy(Buffer->data() + countSlot, &destroyCount, 2);

		// important to remember that because objects are destroyed during the frame
		// and snapshots are packed after the frame, snapshots are not created for objects 
		// on the frame they are destroyed and we can only remember them through destroy data

		// add snapshots
		// create 4 slots for 32 bit count
		Buffer->push_back(0);
		Buffer->push_back(0);
		Buffer->push_back(0);
		Buffer->push_back(0);
		size_t snapSlot = Buffer->size() - 4;
		u32 snapSize = Buffer->size();
#if !_SERVER
		if (KNetInterface_Client* client = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
			for (auto& kv : client->ConnectedPlayers)
				kv.second->CreateSnapshot();
		else
#endif
		for (KPoolable* p : *TDataPool<KNetPlayer>::GetPool())
			dynamic_cast<KNetPlayer*>(p)->CreateSnapshotForPlayer(CompareFrame, playerIndex, Buffer, bStoringReplay);
		for (auto pool : SnapshotDataPools)
		  for (KPoolable* p : *pool)
		    dynamic_cast<KSnapshottable*>(p)->CreateSnapshotForPlayer(CompareFrame, playerIndex, buffer, replay);

		snapSize = Buffer->size() - snapSize;
		memcpy(Buffer->data() + snapSlot, &snapSize, 4);

		// add sounds
		Buffer->push_back(0); // count slot
		countSlot = Buffer->size() - 1;
		u8 soundCount = 0;
		for (const auto& kv : GetNetState()->PendingNetSounds)
		{
			if (kv.first < CompareFrame && CompareFrame > 0) continue;
			for (const KReppedSound& sound : kv.second)
			{
				// sounds have a cutoff frame if they were created with bLowHoldFrames = true
				// prevents clients from getting bombarded with useless sounds after losing a lot of frames
				if (KTime::FrameCount() > sound.CutoffFrame) continue;

				// make sure this player needs this sound
				if (sound.RepMethod == EReplicatedSound::SendIndex)
				{
					if (playerIndex != sound.PlayerIndex) continue;
				}
				else if (sound.RepMethod == EReplicatedSound::SkipIndex)
				{
					if (playerIndex == sound.PlayerIndex) continue;
				}

				BufferSnapshotData((u8*)sound.PackedData, sound.AllocSize);
				soundCount++;

				if (soundCount == 255) break; // 255 wtf
			}
			if (soundCount == 255) break; // double break instead of badass goto statement
		}
		memcpy(Buffer->data() + countSlot, &soundCount, 1);
	}

	dataSize = buffer->size() - dataSize;

	if (replay)
		memcpy(buffer->data() + sizeSlot, &dataSize, 4);
}

void KGameStateSnapshot::BufferSnapshotData(u8* data, u32 byteCount)
{
	Buffer->insert(Buffer->end(), data, data + byteCount);
}
