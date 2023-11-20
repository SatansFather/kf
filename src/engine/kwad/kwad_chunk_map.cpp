#include "kwad_chunk_map.h"

u32 KWadChunk_Map::GetSize()
{
	return Data.size();
}

void KWadChunk_Map::WriteToBuffer(u8* buffer)
{
	memcpy(buffer, Data.data(), Data.size());
}

void KWadChunk_Map::ReadFromBuffer(u8* buffer, u32 size)
{
	Data.resize(size);
	memcpy(Data.data(), buffer, size);
}
