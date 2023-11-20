#pragma once

#include "kwad_chunk.h"

struct KWadChunk_Font : public KWadChunk
{
	TVector<char> Data;

	u32 GetSize() override;
	void WriteToBuffer(u8* buffer) override;
	void ReadFromBuffer(u8* buffer, u32 size) override;
};
