#include "peer.h"
#include "packet.h"
#include "../messages/message_channel.h"
#include "address.h"
#include "../messages/server_welcome.h"
#include "../net_interface.h"

void KNetPeer::SendPacket(class KNetPacket* packet)
{
	i32 success = enet_peer_send(Peer, (u8)packet->Channel, packet->Packet);

	if (KNetInterface* iface = GetNetInterface())
	{
		iface->AddOutBytesThisSecond(packet->PackedSize);
	}
}

void KNetPeer::SendPasswordRequest()
{

}

void KNetPeer::SendServerWelcome()
{
	KServerWelcome::PrepareAndSend(this);
}

void KNetPeer::SendGameState(const u8* data, u32 size)
{
	KNetPacket packet(data, size, EMessageType::S_GameState);
	enet_peer_send(Peer, (u8)EPacketChannel::GameState, packet.Packet); 
}

KNetAddress KNetPeer::GetAddress() const
{
	return { Peer->address.host, Peer->address.port };
}

class KNetPlayer* KNetPeer::GetPlayer()
{
	return (KNetPlayer*)Peer->data;
}

void KNetPeer::DisconnectPeer(u32 reason)
{
	enet_peer_disconnect(Peer, reason);
}

void KNetPeer::ResetPeer()
{
	enet_peer_reset(Peer);
}
