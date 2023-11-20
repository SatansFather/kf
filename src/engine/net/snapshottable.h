#pragma once

#include "kfglobal.h"
#include "engine/math/vec3.h"
#include "snapshot.h"
#include <cstring>

#define MAX_SNAPSHOTS 64

#define SNAP_PROP(type, name, ...) type name; // property will be synced from server to clients
#define SNAP_PROP_TRANSIENT(type, name, ...)  // not stored as a member on the object
#define SNAP_REP_FUNC(func)       // name of function to be called if the value is updated, returns void and takes prop type as arg (with previous data)
#define SNAP_DESTROY              // packed with destroy data
#define SNAP_SEND_OWNER           // only sent to the owner of the object
#define SNAP_SEND_OTHERS          // sent to everyone but the owner of the object
#define SNAP_SEND_VIEWING         // only sent to players viewing this object (owner or spectator)
#define SNAP_FIRST_ONLY           // stops sending after server gets first ack from client for this object
#define SNAP_REPLAY_ONLY          // property is only packed for replays and ignores net
#define SNAP_SKIP_REPLAY          // property ignores replays and is only for net
#define SNAP_SEND_CONDITION(func) // allow a function to decide if this should be sent

class KSnapshottable
{
public:
	u32 NetID = 0;
	u8 ClassID = 0; // look it up once and store it here
	class KSnapshot* HistoryStart = nullptr;

	u32 SnapshottableFrameCreated = 0;

	// server's net frame when it spawned this object, or the frame the client acks when it receives this object
	//u32 ServerFrameCreated = 0;

	// client's local net state NetFrameCount when they spawned this replicated object
	//u32 ClientFrameCreated = 0;

	// index of the owning player
	SNAP_PROP(u8, OwningPlayerIndex = NULL_PLAYER)

	// tells a client that this object was created by the server
	// clients cannot destroy these on their own
	bool bCreatedFromServer = false;

	KSnapshottable();
	virtual ~KSnapshottable();

	virtual void OnNetUpdate() {}

	// stuff here to prevent includes in this header
	void AddDestroyToNetState(UPtr<class KDestroySnapshot> destroy, u32 size);
	void AddBytesToPlayerStateSend(u8 playerIndex, u8* data, u32 byteCount);
	void RemoveBytesFromEndOfPlayerStateSend(u8 playerIndex, u32 byteCount);
	u32 AddCountSlotToPlayerStateSend(u8 playerIndex);
	void AddMemberCountToPlayerStateSend(u8 playerIndex, u32 dataIndex, u8 count);
	void AddToNetState();
	void RemoveFromNetState();

	bool HasObjectAuthority() const;

	void CreateSnapshotForPlayer(u32 lastAcked, u8 playerIndex, TVector<u8>* buffer, bool storingReplay = false);

	// called on client after all new net objects for the frame have been created and unpacked
	// not called on server
	virtual void InitNetObject() {}


	// called on client after ClientData has been fully updated
	//virtual void UnpackSnapshot() {};
	//virtual void UnpackDestroy(KDestroyedNetObject* data, bool repeatDestroy) {};

	virtual void OnNetDestroy() {}

	//virtual KDestroyedNetObject* ReadRemainingDestroyInfo(const u8* data, u32& cursor) = 0;
	void DestroyNetObject(bool recentlyDestroyed = false);

	u32 GetNetID() { return NetID; }
	u8 GetClassID() { return ClassID; }
	void CreateDestroyData();
	void CreateSnapshot();
	virtual void PreCreateSnapshot() {}
};

/*
template <typename SnapshotType, typename DestroyType = KDestroyedNetObject>
class TSnapshottable : public KSnapshottable
{
	static_assert(std::is_base_of<KNetSnapshot, SnapshotType>::value, "SnapshotType must derive from KNetSnapshot");
	static_assert(std::is_base_of<KDestroyedNetObject, DestroyType>::value || std::is_same<KDestroyedNetObject, DestroyType>::value, 
		"DestroyType must derive from KDestroyedNetObject");


public:

	KDestroyedNetObject* ReadRemainingDestroyInfo(const u8* data, u32& cursor) override
	{
		if constexpr (!std::is_same<DestroyType, KDestroyedNetObject>::value)
		{
			// initial destroy data has already been read, subtract it from pointer
			DestroyType* d = (DestroyType*)(data - sizeof(KDestroyedNetObject));

			// adjust cursor for what we've just taken
			cursor += sizeof(DestroyType) - sizeof(KDestroyedNetObject);

			return d;
		}
		return nullptr;
	}
};

*/
