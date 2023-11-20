#include "kwad_chunk_font.h"

u32 KWadChunk_Font::GetSize()
{
	return Data.size();
}

void KWadChunk_Font::WriteToBuffer(u8* buffer)
{
	memcpy(buffer, Data.data(), Data.size());
}

void KWadChunk_Font::ReadFromBuffer(u8* buffer, u32 size)
{
	Data.resize(size);
	memcpy(Data.data(), buffer, size);
}
