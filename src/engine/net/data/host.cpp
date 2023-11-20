#include "host.h"
#include "peer.h"
#include "packet.h"
#include "../net_interface.h"
#include "address.h"
#include "../messages/message_channel.h"
#include "../player.h"
#include "engine/game/local_player.h"

KNetHost::KNetHost(u32 peerCount, u16 port)
{
	ENetAddress addr;
	addr.host = ENET_HOST_ANY;
	addr.port = port; // 1 == client, else server
	Host = enet_host_create(peerCount == 1 ? NULL : &addr, peerCount, (u8)EPacketChannel::NumChannels, 0, 0);
}

KNetHost::~KNetHost()
{
	
}

void KNetHost::QueryEvents(KNetInterface* iface)
{
	ENetEvent evnt;
	while (enet_host_service(Host, &evnt, 0))
	{
		switch (evnt.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				ProcessReceive(&evnt, iface);
				enet_packet_destroy(evnt.packet);
				break;
			}
			case ENET_EVENT_TYPE_CONNECT:
			{
				ProcessConnect(&evnt, iface);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				ProcessDisconnect(&evnt, iface);
				break;
			}
		}
	}
}

UPtr<KNetPeer> KNetHost::InitConnection(KNetAddress* addr)
{
	ENetPeer* p = enet_host_connect(Host, &(addr->Address), (u8)EPacketChannel::NumChannels, 0);
	if (!p) return nullptr;
	UPtr<KNetPeer> peer = std::make_unique<KNetPeer>();
	peer->Peer = p;
	return peer;
}

bool KNetHost::FinalizeConnection()
{
	ENetEvent evnt;
	return enet_host_service(Host, &evnt, 3000) > 0 && evnt.type == ENET_EVENT_TYPE_CONNECT;
}

void KNetHost::Shutdown(KNetPeer* server)
{
	enet_peer_disconnect(server->Peer, 0);

	// wait 3 seconds for disconnect to be acknowledged
	ENetEvent evnt;
	while (enet_host_service(Host, &evnt, 3000) > 0)
	{
		switch (evnt.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				// destroy any incoming packets while disconnecting
				enet_packet_destroy(evnt.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				//OnDisconnect(evnt);
				return;
			}
		}
	}

	// force connection shutdown if we got here
	enet_peer_reset(server->Peer);
}

void KNetHost::ProcessReceive(ENetEvent* evnt, KNetInterface* iface)
{	
	KNetPacket p;
	p.Packet = evnt->packet;
	p.Channel = (EPacketChannel)evnt->channelID;

	iface->AddInBytesThisSecond(p.Packet->dataLength);

	EMessageType type = p.GetMessageType();
	u32 decompSize = p.GetUncompressedSize();
	u32 offset = 5;

	u32 frame = 0;
	if (TypePacksFrame(type))
	{
		offset += 4;
		frame = p.GetFrameNumber();
		if (!AcceptFrame(frame, evnt, iface)) 
			return;
	}

	UPtr<u8[]> decompressed = p.DecompressData(evnt->packet->data, evnt->packet->dataLength - offset);
	if (!decompressed) return; // invalid or outdated packet

	iface->OnReceive((KNetPlayer*)evnt->peer->data, decompressed.get(), decompSize, type, frame);
}

void KNetHost::ProcessConnect(ENetEvent* evnt, KNetInterface* iface)
{
	if (KNetPlayer* p = iface->CreateNewPlayer()) // only server can do this
	{
		evnt->peer->data = p;
		p->NetPeer = new KNetPeer;
		p->NetPeer->Peer = evnt->peer;
		p->StateSendBuffer.reserve(2048);
		iface->OnConnection(p->NetPeer);
	}
	else if (iface->IsClient())
	{
		KNetPeer peer = { evnt->peer };
		iface->OnConnection(&peer);
	}
}

void KNetHost::ProcessDisconnect(ENetEvent* evnt, KNetInterface* iface)
{
	KNetPeer peer = { evnt->peer };
	iface->OnDisconnect(&peer);
}

bool KNetHost::AcceptFrame(u32 frame, ENetEvent* evnt, KNetInterface* iface)
{
	if (iface->IsClient())
	{
		// drop outdated game state frames
		if (KNetPlayer* p = GetLocalPlayer()->LocalNetPlayer)
		{
#if 1
			if (p->LastStateFrameUnpacked >= frame)
				return false;

			if (frame > p->LastStateFrameReceived)
				p->LastStateFrameReceived = frame;
#else
			if (p->LastStateFrameReceived >= frame)
				return false;

			p->LastStateFrameReceived = frame;
#endif
		}
		else return false;
	}
	else
	{
		return frame >= KTime::FrameCount();


		// drop outdated client input frames
		if (frame <= ((KNetPlayer*)evnt->peer->data)->LastInputFrame)
			return false;
		((KNetPlayer*)evnt->peer->data)->LastInputFrame = frame;
	}

	return true;
}

