#pragma once

#include "enet/enet.h"
#include "kfglobal.h"
#include "../messages/message_channel.h"

enum EServerWelcomeResponse
{
	Success,
	NeedsMap
};

struct KNetPeer
{
	ENetPeer* Peer = nullptr;

	void SendPacket(class KNetPacket* packet);

	void SendPasswordRequest();
	void SendServerWelcome();
	void SendGameState(const u8* data, u32 size);
	/*void SendUserChatMessage(const KString& message);
	void SendSystemMessage(const KString& message);
	void SendSystemWarning(const KString& message);
	void SendConsoleMessage(const KString& message);
	void SendConsoleWarning(const KString& message);*/

	class KNetAddress GetAddress() const;
	class KNetPlayer* GetPlayer();

	void DisconnectPeer(u32 reason);
	void ResetPeer();

private:

	//void SendString(const KString& str, EPacketChannel channel, EMessageType type);

};