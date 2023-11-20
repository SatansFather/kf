#pragma once

#include "kwad_chunk.h"

struct KWadChunk_Obj : public KWadChunk
{
	u32 ObjSize = 0, MtlSize = 0;
	KString Obj, Mtl;

	u32 GetSize() override;
	void WriteToBuffer(u8* buffer) override;
	void ReadFromBuffer(u8* buffer, u32 size) override;
};
