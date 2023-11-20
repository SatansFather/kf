#pragma once

#include "enet/enet.h"
#include "kfglobal.h"
#include "../messages/message_channel.h"

struct KNetPacket
{
	ENetPacket* Packet = nullptr;
	EPacketChannel Channel;
	u32 PackedSize = 0;

	KNetPacket() = default;
	KNetPacket(const u8* data, u32 size, EMessageType type);
	~KNetPacket();

	UPtr<u8[]> DecompressData(const u8* data, u32 compressedSize);
	u8* CompressData(const u8* data, u32 srcSize, u32& outSize, EMessageType type) const;

	EMessageType GetMessageType();
	u32 GetFrameNumber();
	u32 GetUncompressedSize();
	u8* GetCompressedAddress();
};