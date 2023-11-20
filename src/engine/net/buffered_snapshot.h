#pragma once

#include "kfglobal.h"

struct KBufferedSnapshot
{
	UPtr<u8[]> Data;
	u32 Size = 0;
	u32 ServerFrame = 0;
	u32 ClientFrame = 0;

	KBufferedSnapshot() = default;
	KBufferedSnapshot(const u8* data, const u32 size, u32 frame)
	{
		Data = std::make_unique<u8[]>(size);
		memcpy(Data.get(), data, size);
		Size = size;
		ServerFrame = frame;
		ClientFrame = KTime::FrameCount();
	}
};