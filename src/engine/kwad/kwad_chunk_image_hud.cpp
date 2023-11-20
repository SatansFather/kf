#include "kwad_chunk_image_hud.h"

u32 KWadChunk_ImageHud::GetSize()
{
	return 8 + Data.size();
}

void KWadChunk_ImageHud::WriteToBuffer(u8* buffer)
{
	memcpy(buffer, &Width, 4);
	memcpy(buffer + 4, &Height, 4);
	memcpy(buffer + 8, Data.data(), Data.size());
}

void KWadChunk_ImageHud::ReadFromBuffer(u8* buffer, u32 size)
{
	memcpy(&Width, buffer, 4);
	memcpy(&Height, buffer + 4, 4);

	Data.resize(size - 8);
	memcpy(Data.data(), buffer + 8, size - 8);
}
