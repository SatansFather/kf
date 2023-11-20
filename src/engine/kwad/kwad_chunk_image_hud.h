#pragma once

#include "kwad_chunk.h"

struct KWadChunk_ImageHud : public KWadChunk
{
	u32 Width = 0;
	u32 Height = 0;
	TVector<char> Data;

	u32 GetSize() override;
	void WriteToBuffer(u8* buffer) override;
	void ReadFromBuffer(u8* buffer, u32 size) override;
};
