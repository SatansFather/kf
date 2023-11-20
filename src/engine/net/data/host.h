#pragma once

#include "enet/enet.h"
#include "kfglobal.h"

struct KNetHost
{
	ENetHost* Host = nullptr;

	KNetHost(u32 peerCount, u16 port = 6668);
	~KNetHost();

	void QueryEvents(class KNetInterface* iface);
	UPtr<class KNetPeer> InitConnection(class KNetAddress* addr);
	bool FinalizeConnection();
	void Shutdown(KNetPeer* server);

private:
	
	void ProcessReceive(ENetEvent* evnt, KNetInterface* iface);
	void ProcessConnect(ENetEvent* evnt, KNetInterface* iface);
	void ProcessDisconnect(ENetEvent* evnt, KNetInterface* iface);

	bool AcceptFrame(u32 frame, ENetEvent* evnt, KNetInterface* iface);

};
