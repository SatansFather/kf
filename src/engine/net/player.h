#pragma once

#include "kfglobal.h"
#include "messages/client_input.h"
#include "snapshottable.h"
#include "game/entity/entity.h"
#include "../game/damage_number.h"
#include "player_ping.h"

// cant directly replicate arrays
struct KReppedPlayerName
{
	u8 NameSize = 0;
	char Name[32];
};

class KNetPlayer : public KSnapshottable,
	public KPoolable
{
	friend class KNetHost;

public:

	class KNetPeer* NetPeer = nullptr;

	SNAP_PROP_TRANSIENT(KReppedPlayerName, ReppedName)
	void GetTransient_ReppedName(KReppedPlayerName& val);
	void SetTransient_ReppedName(KReppedPlayerName& val);
	KString PlayerName = "Player";

	// last client input frame received by the server
	SNAP_PROP(u32, LastInputFrame = 0, SNAP_SEND_OWNER, SNAP_SKIP_REPLAY)

	// server: last state frame the client received, according to their most recent input
	// client: knows that server is aware client received this, client knows it wont get this frame again
	SNAP_PROP(u32, LastAckedFrame = 0, SNAP_SEND_OWNER, SNAP_SKIP_REPLAY)

	// how many input frames the server has missed from the client recently
	SNAP_PROP(u8, TotalFrameDrops = 0, SNAP_SEND_OWNER, SNAP_SKIP_REPLAY)

	SNAP_PROP(i32, Score = 0)
	SNAP_PROP(i32, Frags = 0)
	SNAP_PROP(i32, Deaths = 0)
	SNAP_PROP(i32, Damage = 0)
	SNAP_PROP_TRANSIENT(u8, PackedPing = 0)
	void GetTransient_PackedPing(u8& val);
	void SetTransient_PackedPing(u8& val);
	u8 PingRequestID = 0;
	TMap<u8, KTimePoint> PendingPingRequests;
	KPlayerPing PlayerPing;
	u16 ClientPing = 0;

	// TODO need a single replicated game state entity with global information like this
	SNAP_PROP_TRANSIENT(bool, bUpdateScoreboard = false, SNAP_SEND_OWNER)
	void GetTransient_bUpdateScoreboard(bool& val);
	void SetTransient_bUpdateScoreboard(bool& val);

	// last state frame the client has received, may still be packed
	u32 LastStateFrameReceived = 0;

	// true when server calls TickObjects on player pool from match frame tick
	static bool bMatchTick;

	// this client is loaded into the match and is able to receive game state updates
	bool bReceivingGameState = false;

	// last state frame the client began to unpack, updated before unpacking snapshots
	// server sees this as the client's current interp frame
	u32 LastStateFrameUnpacked = 0;

	// client sets to last state frame after it finishes unpacking
	// used during unpack to see if we've already applied certain data
	u32 LastFinishedUnpackedFrame = 0;

	// the frame this client finished loading the map
	u32 FinishLoadingFrame = 0;

	// client needs to adjust their frame count to match the server upon connecting, only once
	bool bAdjustedFrame = false;

	// difference between client's local frame and the server frame theyre on
	// set by server and told to client
	f32 ClientFrameDiff = 0;

	// last calculated frame serverside comp count
	u8 CurrentCompFrames = 0;

	// preallocate one buffer for all game state packets
	// resize if needed
	TVector<u8> StateSendBuffer;

	// client input history per frame, for server
	TMap<u32, KBufferedClientInput> ClientInput;
	
	// damage numbers the server needs to send to this player (or their spectators)
	TVector<KReppedDamageNumber> OutDamageNumbers;

	// client input history per frame, for client to resend
	//TVector<KClientInputFrame> LocalClientInput;

	TObjRef<KEntity> ControlledEntity;

	TVector<u32> LostInputFrames;

	TVector<KClientInputFrame> StoredLocalInput;

	TMap<u32, KTimePoint> TimeOfFrames;

	// spectated player
	u8 ViewedPlayerIndex = NULL_PLAYER;

	// server will flag this when receiving a spawn request
	// will act on it when running the next frame
	bool bPendingSpawnRequest = false;

	bool bSpectatingLastFrame = true;

public:

	KNetPlayer();
	~KNetPlayer();

	void OnNetUpdate() override;

	// server updates net through here
	void Tick() override;

	void SetReceivingState(bool allow) { bReceivingGameState = allow; }
	bool IsReceivingState() const { return bReceivingGameState; }

	bool CreatedThisFrame() { return false; }

	KBufferedClientInput* GetMostRecentInput(u32& frameNumber);
	void AddClientInput(const KClientInputFrame& frame);

	void InitNetObject() override;

	void BufferSnapshotData(u8* data, u32 byteCount);
	void PopSnapshotData(u32 bytesToRemove);

	bool IsLocalPlayer() const;
	bool IsViewedPlayer() const;

	void BuildInputPacket(UPtr<u8[]>& inputData, u32& packedSize);
	void ClearOldStoredInput();

	bool IsSpectating() const { return ViewedPlayerIndex != OwningPlayerIndex; }

	void NewMapReset();

private:

	void AddCurrentInputFrame();
	void SendPingRequest();
	void RemoveOldData();
	void SendGameState();
};

KNetPlayer* GetLocalNetPlayer();