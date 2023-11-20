#include "kwad_chunk_sound.h"

u32 KWadChunk_Sound::GetSize()
{
	return Data.size();
}

void KWadChunk_Sound::WriteToBuffer(u8* buffer)
{
	memcpy(buffer, Data.data(), Data.size());
}

void KWadChunk_Sound::ReadFromBuffer(u8* buffer, u32 size)
{
	Data.resize(size);
	memcpy(Data.data(), buffer, size);
}
