#include "snapshottable.h"
#include "player.h"
#include "net_interface.h"
#include "state.h"
#include "game/entity/entity.h"
#include "client.h"
#include "snapshot.h"
#include "../../game/entity/pickups/pickup_health.h"

KSnapshottable::KSnapshottable()
{
	SnapshottableFrameCreated = KTime::FrameCount();
}

KSnapshottable::~KSnapshottable()
{
	KSnapshot::RemoveObjectFromHistory(ClassID, NetID);
	RemoveFromNetState();
}

void KSnapshottable::AddDestroyToNetState(UPtr<KDestroySnapshot> destroy, u32 size)
{
	if (GetNetState())
		GetNetState()->DestroyedNetItems[KTime::FrameCount()].push_back({ std::move(destroy), size } );
}

void KSnapshottable::AddBytesToPlayerStateSend(u8 playerIndex, u8* data, u32 byteCount)
{
	if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(playerIndex))
		p->BufferSnapshotData(data, byteCount);
}

void KSnapshottable::RemoveBytesFromEndOfPlayerStateSend(u8 playerIndex, u32 byteCount)
{
	if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(playerIndex))
		p->PopSnapshotData(byteCount);
}

u32 KSnapshottable::AddCountSlotToPlayerStateSend(u8 playerIndex)
{
	if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(playerIndex))
	{
		p->StateSendBuffer.push_back(0);
		return p->StateSendBuffer.size() - 1;
	}
	return MAX_U32;
}

void KSnapshottable::AddMemberCountToPlayerStateSend(u8 playerIndex, u32 dataIndex, u8 count)
{
	if (KNetPlayer* p = GetNetInterface()->GetPlayerFromIndex(playerIndex))
		p->StateSendBuffer[dataIndex] = count;
}

void KSnapshottable::AddToNetState()
{
	KNetState* state = GetNetState();
	if (!state) return;

	if (IsNetServer())
	{
		if (ClassID == KSnapshot::GetPlayerClassID())
		{
			// players are not added to the regular net state map
			// net state objects are destroyed with the match but players are persistent
			NetID = 0xFFFFFF00;
			NetID |= (dynamic_cast<KPoolable*>(this))->Poolable_GetIndex();
		}
		else if (NetID == 0)
		{
			if (KNetState* state = GetNetState())
			{
				NetID = state->NextNetID;
				state->AddReplicatedObject(this);
				state->NextNetID++;
			}
		}
	}
	else
	{
		// client needs to assign a net ID to their local projectiles if recording
		// TODO this breaks IsMyNetProjectile
		if (IsWritingReplay() && NetID == 0)
		{
			NetID = state->NextNetID;
			NetID |= 0x40000000;
			state->NextNetID++;
		}
	}
}

void KSnapshottable::RemoveFromNetState()
{
	if (GetNetState())
	{
		GetNetState()->ClearReplicatedObject(NetID);
#if !_SERVER
		// we're considered a client when we start loading into the server
		// so net state and net player need to be checked because theyre invalid at that point
		if (IsNetClient() && GetNetState() && GetLocalNetPlayer()) 
			GetNetState()->RecentClientDestroys[GetLocalNetPlayer()->LastStateFrameReceived].push_back(NetID);
#endif
	}
}

bool KSnapshottable::HasObjectAuthority() const
{
	return IsNetAuthority() || !bCreatedFromServer;
}

void KSnapshottable::DestroyNetObject(bool recentlyDestroyed)
{
	bCreatedFromServer = false; // allow client to destroy this
	if (KEntity* ent = dynamic_cast<KEntity*>(this))
	{
		if (recentlyDestroyed) ent->Poolable_Destroy();
		else ent->DestroyEntity(); // dont do normal destroy stuff again
	}
	else if (KNetPlayer* p = dynamic_cast<KNetPlayer*>(this))
	{
		if (IsNetServer())
		{
			p->Poolable_Destroy();
		}
#if !_SERVER
		else if (KNetInterface_Client* client = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		{
			//GetNetState()->ClearReplicatedObject(NetID);
			client->ClearPlayerByIndex(OwningPlayerIndex);
		}
#endif
	}
}

void KSnapshottable::CreateDestroyData()
{
	if (!IsNetServer()) return;

	if (ClassID == 0)
	{
		ClassID = KSnapshot::LookUpClassID(typeid(*this));
		AddToNetState();
		HistoryStart = KSnapshot::GetHistoryForObject(ClassID, NetID);
	}

	K_ASSERT(ClassID != 0, "destroying entity with a class ID that is still 0");

	UPtr<KDestroySnapshot> destroy = KDestroySnapshot::FillDestroy(this);
	AddDestroyToNetState(std::move(destroy), KDestroySnapshot::GetSize(ClassID));
}

void KSnapshottable::CreateSnapshot()
{
	//if (NetID == 0) return;

	//K_ASSERT(IsNetServer(), "CreateSnapshots called from client");
	u32 frame = KTime::FrameCount();

	if (!HistoryStart)
	{
		ClassID = KSnapshot::LookUpClassID(typeid(*this));
		if (HasObjectAuthority()) AddToNetState();
		HistoryStart = KSnapshot::GetHistoryForObject(ClassID, NetID);
	}

	PreCreateSnapshot();
	KSnapshot* thisFrame = HistoryStart->GetHistoryAtFrame(frame);
	thisFrame->PackObject(this);
}

void KSnapshottable::CreateSnapshotForPlayer(u32 lastAcked, u8 playerIndex, TVector<u8>* buffer, bool storingReplay)
{
	//if (NetID == 0) return;

	KSnapshot* thisFrame = HistoryStart->GetHistoryAtFrame(KTime::FrameCount());
	KSnapshot* lastFrame = HistoryStart->GetHistoryAtFrame(lastAcked);
	if (!lastFrame)
	{
		// if this is a new player or the last ack is too old, send everything
	}

	u32 countIndex = 0;
	u32 packedCount = 0;

	// add object NetID
	u32 netID = NetID;

	bool wrote = false;

	// iterate properties to see what we should pack
	for (u8 i = 0; i < thisFrame->MemberCount(); i++)
	{
		if (lastFrame && !thisFrame->ClientNeedsProperty(i, playerIndex, this, lastAcked, storingReplay)) continue;

		bool change = true;

		// get the member offset and member size
		u8* addr = nullptr;
		u8 size = 0;
		thisFrame->GetAddressAndSize(i, addr, size);

		// compare current frame with the last acked frame
		if (lastFrame)
		{
			u8* prev = nullptr;
			lastFrame->GetAddressAndSize(i, prev, size);
			change = std::memcmp(addr, prev, size) != 0;
		}

		if (change)
		{
			if (!wrote)
			{
				buffer->push_back(0);
				countIndex = buffer->size() - 1;			
				buffer->insert(buffer->end(), (u8*)&netID, (u8*)(&netID) + 4);
				buffer->insert(buffer->end(), (u8*)&ClassID, (u8*)(&ClassID) + 1);
				wrote = true;
			}
			 
			// pack index of value
			buffer->insert(buffer->end(), (u8*)&i, (u8*)(&i) + 1);

			// pack changed value
			buffer->insert(buffer->end(), (u8*)addr, (u8*)(addr) + size);

			packedCount++;
		}
	}

	if (wrote) (*buffer)[countIndex] = packedCount;
}
