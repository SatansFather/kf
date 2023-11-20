#include "state.h"
#include "engine/game/match.h"
#include "player.h"
#include "client.h"
#include "../game_instance.h"
#include "engine/audio/audio.h"
#include "snapshot.h"

KNetState::KNetState()
{
	// start tracking objects that existed before this
	//for (const auto& kv : *KDataPoolBase::GetSnapshotPoolMap())
	//	kv.second->AddExistingObjectsToNetState();

	// player objects didnt go away, we need to add them to the new replicated object map
	if (IsNetServer())
	{
		if (auto pool = TDataPool<KNetPlayer>::GetPool())
		{
			for (KPoolable* p : *pool)
			{
				KNetPlayer* player = dynamic_cast<KNetPlayer*>(p);
				player->SnapshottableFrameCreated = 0;
				player->AddToNetState();
			}
		}		
	}
	
	memset(SnapshotReceiveFrames.Frames, 0, sizeof(u32) * 20);
}

KNetState::~KNetState() {}

bool KNetState::HasReplicatedObject(u32 id)
{
	return ReplicatedObjects.contains(id);
}

void KNetState::AddReplicatedObject(class KSnapshottable* obj)
{
	ReplicatedObjects[obj->NetID] = obj;
}

class KSnapshottable* KNetState::GetReplicatedObject(u32 netID)
{
	if (ReplicatedObjects.contains(netID))
		return ReplicatedObjects[netID];
	return nullptr;
}

void KNetState::ClearReplicatedObject(u32 netID)
{
	ReplicatedObjects.erase(netID);
}

void KNetState::ClearPendingPlayers()
{
	for (KNetPlayer* player : PendingPlayers)
		delete player;

	PendingPlayers.clear();
}

bool KNetState::NetIdWasRecentlyDestroyed(u32 id) const
{
	for (const auto& kv : RecentClientDestroys)
	  if (VectorContains(kv.second, id))
		return true;

	return false;
}

void KNetState::ProcessReppedSound(KReppedSound& sound, const KSoundProperties& props)
{
	u32 frame = KTime::FrameCount();

	sound.SetVolume(props.Volume);
	sound.SetAttenuation(props.MaxDistance);
	sound.SetFrame(frame);
	sound.SetPlayerIndex(props.PlayerIndex);
	sound.SetRepMethod(props.ReplicationMethod);
	if (props.bLowHoldFrames) sound.CutoffFrame = frame + 45;

	sound.PreparePack();

	PendingNetSounds[frame].push_back(std::move(sound));
}

f32 KNetState::GetInterpAlpha() const
{
	const u32 sinceLast = FramesSinceLastSnapshot;
	const u32 snapInterval = KMax(SnapshotInterpFrames, 1u);

	f32 alpha = ((f32)sinceLast + 1.f) / (f32)snapInterval;
	if (alpha != alpha) alpha = 1; // nan check
	return KSaturate(alpha);	
}

void KNetState::BufferSnapshotData(const u8* data, const u32 dataSize, const u32 frameNumber)
{
	// frames might have arrived out of order
	// a big enough buffer will allow insertion
	auto it = SnapshotBuffer.begin();
	while (it != SnapshotBuffer.end())
	{
		if (it->ServerFrame > frameNumber)
			break;
			
		std::advance(it, 1);
	}

	SnapshotBuffer.insert(it, KBufferedSnapshot(data, dataSize, frameNumber));
	SnapshotReceiveFrames.AddFrame();
}

void KNetState::UnpackNextSnapshot()
{

#if !_SERVER
	FramesSinceLastSnapshot++;
	if (FramesSinceLastSnapshot < SnapshotInterpFrames) return;

	if (SnapshotBuffer.size() == 0)
	{
		// TODO we should definitely have something but we dont
		// adjust something and yell at the player for having shitty internet

		// increase buffer size
		// once were stable again, reduce buffer size by x
		// then subtract x from the following SnapshotInterpFrames

		return;
	}

	LockConfig();
	const u32 bufferSize = GetUserConfig()->Net.SnapshotBuffer;
	UnlockConfig();

	// TODO ping fluctuations
	// ping dropping leads to a big snapshot buffer and everything is still delayed
	// ping increase shows hitches, might be worth changing speeds

	// ready to unpack a new one
	const KBufferedSnapshot& nextSnapshot = SnapshotBuffer.front();

	KNetPlayer* p = GetLocalNetPlayer();
	KNetInterface_Client* client = dynamic_cast<KNetInterface_Client*>(GetNetInterface());
#if 1

	InterpStartFrame = ServerFrameOfLastUnpackedSnapshot;
	FramesSinceLastSnapshot = 0;

	SnapshotsProcessedThisFrame = 0;

	while (SnapshotBuffer.size() > bufferSize)
	{
#define bufferway 1
#if bufferway
		const KBufferedSnapshot& snap = SnapshotBuffer.front();
		p->LastStateFrameUnpacked = snap.ServerFrame;

		client->ProcessGameState(snap.Data.get(), snap.Size);
		SnapshotInterpFrames = ServerFrameOfLastUnpackedSnapshot == 0 ? 0 :
			snap.ServerFrame - ServerFrameOfLastUnpackedSnapshot;
		ServerFrameOfLastUnpackedSnapshot = snap.ServerFrame;

		InterpTargetFrame = snap.ServerFrame;

		SnapshotBuffer.pop_front();

		SnapshotsProcessedThisFrame++;

		// sometimes unpack two in one frame and none the next
		//if (SnapshotBuffer.size() <= bufferSize + 1)
		//	break;
#else
		const KBufferedSnapshot& snap = SnapshotBuffer.front();
		p->LastStateFrameUnpacked = snap.ServerFrame;
		client->ProcessGameState(snap.Data.get(), snap.Size);

		SnapshotInterpFrames = ServerFrameOfLastUnpackedSnapshot == 0 ? 0 :
			snap.ServerFrame - ServerFrameOfLastUnpackedSnapshot;
		ServerFrameOfLastUnpackedSnapshot = snap.ServerFrame;

		InterpTargetFrame = snap.ServerFrame;

		SnapshotBuffer.pop_front();
#endif
	}

#else

	// make sure we allow a big enough buffer
	if (KTime::FrameCount() - nextSnapshot.ClientFrame <= bufferSize)
		return;

	FramesSinceLastSnapshot = 0;

	SnapshotInterpFrames = ServerFrameOfLastUnpackedSnapshot == 0 ? 0 :
		nextSnapshot.ServerFrame - ServerFrameOfLastUnpackedSnapshot;
	ServerFrameOfLastUnpackedSnapshot = nextSnapshot.ServerFrame;

	if (SnapshotBuffer.size() > bufferSize + 1 && SnapshotInterpFrames > 0)
		SnapshotInterpFrames--;

	p->LastStateFrameUnpacked = nextSnapshot.ServerFrame;
	
	dynamic_cast<KNetInterface_Client*>(GetNetInterface())->ProcessGameState(
		nextSnapshot.Data.get(), nextSnapshot.Size);

	SnapshotBuffer.pop_front();

#endif

	if (p->TimeOfFrames.contains(p->LastStateFrameUnpacked))
	{
		GetNetInterface()->NetStats.ActualDelay = KTime::Since(p->TimeOfFrames[p->LastStateFrameUnpacked]);
		//CHATLOG(SnapshotBuffer.size(), 1, 1, 0);
		MapRemoveTo_Inclusive(p->TimeOfFrames, p->LastStateFrameUnpacked);
	}
#endif
}

u32 KNetState::GetCurrentInterpFrame() const
{
	//const u32 interpFrameB = ServerFrameOfLastUnpackedSnapshot;
	//const u32 interpFrameA = interpFrameB - KMax(SnapshotInterpFrames, 1u);
	//const f32 alpha = GetInterpAlpha();
	//return std::round(KLerp(interpFrameA, interpFrameB, alpha));
	const f32 alpha = GetInterpAlpha();
	return std::round(KLerp(InterpStartFrame, InterpTargetFrame, alpha));
}

u32 KNetState::GetNumBufferedSnapshotFrames() const
{
	const KBufferedSnapshot& last = SnapshotBuffer.back();
	return 0;
}

KNetState* GetNetState()
{
	if (GetGameMatch()) return GetGameMatch()->GetNetState();
	return nullptr;
}
