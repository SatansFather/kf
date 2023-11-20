#pragma once

#include "kfglobal.h"

#pragma pack(push, 1)
struct KWadChunk
{
	KString Name;
	u8 FileType = 0;

	virtual u32 GetSize() = 0;
	virtual void WriteToBuffer(u8* buffer) = 0;
	virtual void ReadFromBuffer(u8* buffer, u32 size) = 0;
};
#pragma pack(pop)