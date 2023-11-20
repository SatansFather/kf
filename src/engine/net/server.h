#pragma once

#include "net_interface.h"
#include "server_config.h"

class KNetInterface_Server : public KNetInterface
{
public:

	// keep track of the lowest acked frame from clients each update
	// lets us know how long to hold onto stuff
	u32 LowestAckedFrame = MAX_U32;

	KServerConfig Config;
	KTimePoint LastMasterUpdateTime;

	bool bUpdateClientScoreboards = true;

	KNetInterface_Server(u16 port = 6668);
	~KNetInterface_Server();

	class KNetPlayer* CreateNewPlayer() override;

	class KNetPlayer* GetPlayerFromIndex(u8 index) override;
	void OnReceive(class KNetPlayer* sender, const u8* data, u32 dataSize, EMessageType type, u32 frameNumber) override;
	void OnConnection(class KNetPeer* peer) override;
	void OnDisconnect(class KNetPeer* peer) override;
	void SendFrame() override;
	void SendChatMessage(const KString& msg, KNetPeer* peer = nullptr, EMessageType type = EMessageType::S_UserChat) override;

	void PackPlayerInfo(TVector<KPlayerScoreInfo>& info) override;

	void NotifyClientsMapChange(u8 hash[16]);

	// first byte of data is size, rest is text
	void ParsePlayerName(class KNetPlayer* player, const u8* data);

	void FlagPendingScoreUpdate() override;

	void UpdateMasterServer() override;
};