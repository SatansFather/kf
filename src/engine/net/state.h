#pragma once

#include "kfglobal.h"
#include "replicated_sound.h"
#include "../collision/broadphase/grid_cell.h"
#include "snapshottable.h"
#include "../collision/broadphase/cell_map.h"
#include "buffered_snapshot.h"

#define SNAPSHOT_INTERVAL 1

class KNetState
{
public:

	u32 NextNetID = 1;
	TMap<u32, class KSnapshottable*> ReplicatedObjects;

	// players that have been created from a state update
	// waiting for all members so we can move them to interface map
	TVector<class KNetPlayer*> PendingPlayers;

	// serverside
	// frame number and the net IDs that were destroyed that frame
	// keys are deleted when the client with the lowest acked frame has received that frame

	// store size here so we dont need a virtual GetSize() funtion for destroyed net object structs
	struct DestroyNetData { UPtr<class KDestroySnapshot> Object; u32 Size; }; 
	TMap<u32, TVector<DestroyNetData>> DestroyedNetItems;

	// sounds played by the server that it wants to send to clients
	TMap<u32, TVector<KReppedSound>> PendingNetSounds;

	// clients will tend to receive destroyed items even after theyve acked the frame, until the server receives that ack
	// they will store all recently destroyed net objects in this map with <frame, IDs>
	// this prevents them from trying to create and re-destroy the same object
	TMap<u32, TVector<u32>> RecentClientDestroys;

	KCellMap PositionSnapshots[MAX_SNAPSHOTS];

	bool bReceivingUpdates = false;

	// snapshots are not unpacked immediately, need a few frames of buffer time for smooth updates
	TList<KBufferedSnapshot> SnapshotBuffer;

	// how many frames since we applied a buffered snapshot to be the most recently unpacked
	u32 FramesSinceLastSnapshot = 0;

	// number of frames to interpolate to the most recent snapshot
	u32 SnapshotInterpFrames = 0;

	// current frame between last received and last unpacked
	// used to tell the server how far behind we are so it can compensate lag accurately
	u32 CurrentInterpFrame = 0;

	u32 InterpStartFrame = 0;
	u32 InterpTargetFrame = 0;
	u32 ServerFrameOfLastUnpackedSnapshot = 0;

	u32 SnapshotsProcessedThisFrame = 0;

	// frame numbers we received the last 20 snapshots on
	// expect them the be 3 frames apart
	// if more than 3 we need to increase our buffer size and wait longer to unpack
	struct  
	{
		u32 Frames[20];
		u32 Index = 0;

		u32 GetLargestReceiveDiff()
		{
			u32 largest = 0;
			for (u32 i = 0; i < 19; i++)
			{
				u32 index = i + Index;
				index %= 20;

				u32 nextIndex = index + 1;
				nextIndex %= 20;

				if (Frames[index] == 0 || Frames[nextIndex == 0])
				// we havent received 20 snapshots yet and reached the end of the Frames buffer
					break;

				u32 diff = Frames[nextIndex] - Frames[index];
				if (diff > largest)
					largest = diff;
			}
			return largest;
		}
		void AddFrame()
		{
			Frames[Index] = KTime::FrameCount();
			Index++;
			Index %= 20;
		}
	} SnapshotReceiveFrames;
	

	KNetState();
	~KNetState();

	bool HasReplicatedObject(u32 id);
	void AddReplicatedObject(class KSnapshottable* obj);
	class KSnapshottable* GetReplicatedObject(u32 netID);
	void ClearReplicatedObject(u32 netID);
	void ClearPendingPlayers();
	bool NetIdWasRecentlyDestroyed(u32 id) const;

	void ProcessReppedSound(struct KReppedSound& sound, const struct KSoundProperties& props);

	f32 GetInterpAlpha() const;

	void BufferSnapshotData(const u8* data, const u32 dataSize, const u32 frameNumber);
	void UnpackNextSnapshot();
	u32 GetCurrentInterpFrame() const;

	// get the total number of frames between right now and the end of our snapshot buffer
	u32 GetNumBufferedSnapshotFrames() const;
};

KNetState* GetNetState();