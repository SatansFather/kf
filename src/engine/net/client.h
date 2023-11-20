#pragma once

#if !_SERVER

#include "net_interface.h"

enum class EConnectionAttemptResult
{
	Pending,
	Failed,
	Success
};

class KNetInterface_Client : public KNetInterface
{
	friend class KNetPlayer;

	// players mapped by their index in the server's data pool
public:
	TMap<u32, KNetPlayer*> ConnectedPlayers;
private:
	UPtr<class KNetPeer> Server;

	EConnectionAttemptResult AttemptResult = EConnectionAttemptResult::Pending;

	f32 TimeDilation = 1;

	u32 NetDilationFramesRemaining = 0;
	u32 LastNetStabilizeFrame = 0;
	i32 NetBruteForceCorrection = 0;

	KString PendingServerMap = "";

public:

	KNetInterface_Client();
	~KNetInterface_Client();

	bool HasConnectionResult() const { return AttemptResult != EConnectionAttemptResult::Pending; }
	bool WasConnectionSuccessful() const { return AttemptResult == EConnectionAttemptResult::Success; }

	void AttemptConnection(class KNetAddress addr);
	
	KString GetServerIP() const;

	//KNetPlayer* GetPlayerByIndex(u8 index);
	void ClearPlayerByIndex(u8 index);

	class KNetPlayer* CreatePlayerAtIndex(u8 index) override;
	void AddPlayerToMap(class KNetPlayer* player);
	void AdjustTiming() override;
	f32 GetTimeDilation() override { return TimeDilation; }
	
	void ProcessGameState(const u8* data, u32 dataSize);
	void NotifyServerLoadedMap();

	void ProcessUpdates() override;

#if _DEV
	void RequestServerQuit();
#endif	

	class KNetPlayer* GetPlayerFromIndex(u8 index) override;
	void OnReceive(class KNetPlayer* sender, const u8* data, u32 dataSize, EMessageType type, u32 frameNumber) override;
	void OnConnection(class KNetPeer* peer) override;
	void OnDisconnect(class KNetPeer* peer) override;
	void SendFrame() override;
	void SendChatMessage(const KString& msg, KNetPeer* peer = nullptr, EMessageType type = EMessageType::S_UserChat) override;

	void PackPlayerInfo(TVector<KPlayerScoreInfo>& info) override;

	void ClearConnectedPlayers();

	class KSnapshottable* GetReplicatedObject(u32 netID);
	bool HasReplicatedObject(u32 netID);

	void SetPendingServerMap(const KString& map);
	void FollowServerMap();
	bool HasPendingServerMap() override { return !PendingServerMap.IsEmpty(); }

	void CallServerMap(const KString& map);
};

#endif