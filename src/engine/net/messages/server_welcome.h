#pragma once

#include "kfglobal.h"

#pragma pack(push, 1)

// initial data sent to client upon successful connection
class KServerWelcome
{
	u8 MapHash[16];
	u8 NetPlayerIndex = 0;
	u32 NetPlayerID = 0;

public:

	static void PrepareAndSend(class KNetPeer* peer);
	static bool ReceiveAndProcess(const KServerWelcome& data);
};

#pragma pack(pop)