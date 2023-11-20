#pragma once

#include "kfglobal.h"
#include "messages/message_channel.h"
#include "net_stats.h"
#include "game/player_score_info.h"

#define DEFAULT_PORT 6668
#define PEER_COUNT 255

// defined as flags so for server + dedicated
struct ENetInterfaceType { enum 
{
	Client = 1, Server = 2, Dedicated = 4
}; };

class KNetInterface
{
protected:

	u8 InterfaceType;
	UPtr<class KNetHost> Host;
	bool bIsShuttingDown = false;
	bool bIsWelcomed = false;

	u32 InBytesThisSecond = 0;
	u32 OutBytesThisSecond = 0;

public:

	bool bPendingScoreUpdate = true;
	bool bPendingNameChange = false;
	KNetStats NetStats;
	
	virtual ~KNetInterface();

	bool IsClient() const { return InterfaceType == ENetInterfaceType::Client; }
	bool IsServer() const { return InterfaceType & ENetInterfaceType::Server; }
	bool IsDedicated() const { return InterfaceType & ENetInterfaceType::Dedicated; }

	bool IsShuttingDown() const { return bIsShuttingDown; }

	bool InitEnet();

	virtual void ProcessUpdates();

	// only server does this, clients will create their player object from server state data
	virtual class KNetPlayer* CreateNewPlayer() { return nullptr; }

	virtual class KNetPlayer* CreatePlayerAtIndex(u8 index) { return nullptr; }
	virtual void AddPlayerToMap(class KNetPlayer* player) {}
	virtual void AdjustTiming() {}
	virtual f32 GetTimeDilation() { return 1; }

	virtual class KNetPlayer* GetPlayerFromIndex(u8 index) = 0;
	virtual void OnReceive(class KNetPlayer* sender, const u8* data, u32 dataSize, EMessageType type, u32 frameNumber) = 0;
	virtual void OnConnection(class KNetPeer* peer) = 0;
	virtual void OnDisconnect(class KNetPeer* peer) = 0;
	virtual void SendFrame() = 0;
	virtual void SendChatMessage(const KString& msg, KNetPeer* peer = nullptr, EMessageType type = EMessageType::S_UserChat) = 0;

	virtual void PackPlayerInfo(TVector<KPlayerScoreInfo>& info) = 0;

	void AddPlayerToScoreInfo(class KNetPlayer* player, TVector<KPlayerScoreInfo>& info);

	bool IsWelcomed() const { return bIsWelcomed; }

	virtual void FlagPendingScoreUpdate();

	virtual void UpdateMasterServer() {}

	virtual bool HasPendingServerMap() { return false; }

	void AddInBytesThisSecond(u32 count);
	void AddOutBytesThisSecond(u32 count);

#if !_SERVER
	UPtr<u8[]> GetLocalPlayerNameAsData(u8& size);
#endif
};

KNetInterface* GetNetInterface();