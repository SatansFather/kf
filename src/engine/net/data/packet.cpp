#include "packet.h"
#include "zlib.h"
#include "../messages/message_channel.h"

static EPacketChannel ChannelFromType(EMessageType type)
{
	switch (type)
	{
		case EMessageType::C_Input:
		case EMessageType::S_GameState:
			return EPacketChannel::GameState;
		case EMessageType::S_Welcome:
		case EMessageType::C_LoadedMap:
		case EMessageType::C_ConnectionAck:
			return EPacketChannel::ServerWelcome;
		case EMessageType::S_NeedPassword:
			return EPacketChannel::Password;
		case EMessageType::S_UserChat:
		case EMessageType::C_ChatMessage:
			return EPacketChannel::UserChatMessage;
		case EMessageType::S_Message:
		case EMessageType::S_MessageWarning:
		case EMessageType::S_MessageError:
			return EPacketChannel::SystemMessage;
		case EMessageType::S_ConsoleMessage:
		case EMessageType::S_ConsoleWarning:
		case EMessageType::S_ConsoleError:
			return EPacketChannel::ConsoleMessage;
	}
	return EPacketChannel::MiscChannel;
}

KNetPacket::KNetPacket(const u8* data, u32 size, EMessageType type)
{
	if (size == 0) return;
	u8* compressed = CompressData(data, size, PackedSize, type);

	Packet = enet_packet_create(compressed, PackedSize, 
		//ENET_PACKET_FLAG_NO_ALLOCATE | 
		(TypeIsReliable(type) ? ENET_PACKET_FLAG_RELIABLE : 0));

	Channel = ChannelFromType(type);

	/*if (!Packet) */delete[] compressed;
}

KNetPacket::~KNetPacket()
{
	if (Packet)
	{
		
		//delete[] Packet->data;
		//enet_free(Packet);
	}
}

UPtr<u8[]> KNetPacket::DecompressData(const u8* data, u32 compressedSize)
{
	UPtr<u8[]> decompressed = std::make_unique<u8[]>(GetUncompressedSize());

	uLongf sz = GetUncompressedSize();
	// TODO if packets are small enough, compression just makes the send size bigger
	//LOG("-----------------------");
	//LOG(GetUncompressedSize());
	//LOG(compressedSize);
	int ret = uncompress(decompressed.get(), &sz, GetCompressedAddress(), compressedSize);
	K_ASSERT(ret == Z_OK, "could not decompress data: " + KString(ret));
	K_ASSERT(sz == GetUncompressedSize(), "packet uncompressed size was incorrect");
	return decompressed;
}

u8* KNetPacket::CompressData(const u8* data, u32 srcSize, u32& outSize, EMessageType type) const
{
	uLongf destLen = srcSize * 1.5 + 12 + 5;
	
	u8 offset = 5;
	if (TypePacksFrame(type))
	{
		// dont compress frame number so we dont waste time decompressing old ones
		offset = 9;
		destLen += 4;
	}

	u8* compressed = new u8[destLen];

	u8 t = (u8)type;
	memcpy(compressed, &t, 1);
	memcpy(compressed + 1, &srcSize, 4);

	if (type == EMessageType::S_GameState)
	{
		u32 num = KTime::FrameCount();
		memcpy(compressed + 5, &num, 4);
	}

	int ret = compress2(compressed + offset, &destLen, data, srcSize, Z_BEST_SPEED);
	K_ASSERT(ret == Z_OK, "could not compress data");

	outSize = destLen + offset;
	return compressed;
}

EMessageType KNetPacket::GetMessageType()
{
	return EMessageType(*(u8*)(Packet->data));
}

u32 KNetPacket::GetFrameNumber()
{
	if (TypePacksFrame(GetMessageType()))
		return *(u32*)(Packet->data + 5);

	return 0;
}

u32 KNetPacket::GetUncompressedSize()
{
	return *(u32*)(Packet->data + 1);
}

u8* KNetPacket::GetCompressedAddress()
{
	u32 offset = 5;
	if (TypePacksFrame(GetMessageType())) 
		offset += 4;
	return Packet->data + offset;
}
