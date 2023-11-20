#include "kwad_chunk_image.h"
#include "engine/render/surface2d.h"
#include "kwad_file.h"

#if !_SERVER
void KWadChunk_Image::CreateFromSurface(class KSurface2D* surface)
{
	FileType = EWadFileChunkType::Image;
	Name = surface->GetName();
	Width = surface->GetPaddedWidth();
	Height = surface->GetPaddedHeight();
	DisplayWidth = surface->GetDisplayWidth();
	DisplayHeight = surface->GetDisplayHeight();
	PadMethod = surface->GetPadMethod();
	bHasTransparency = surface->HasTransparency();
	Data = std::make_unique<FColor8[]>(Width * Height);
	memcpy(Data.get(), surface->GetData(), Width * Height * 4);
}
#endif

u32 KWadChunk_Image::GetSize()
{
	return sizeof(BufferedChunk) + (Width * Height * 4);
}

void KWadChunk_Image::WriteToBuffer(u8* buffer)
{
	BufferedChunk c;
	c.CopyFrom(this);
	memcpy(buffer, &c, sizeof(BufferedChunk));
	memcpy(buffer + sizeof(BufferedChunk), Data.get(), Width * Height * 4);
}

void KWadChunk_Image::ReadFromBuffer(u8* buffer, u32 size)
{
	BufferedChunk c;
	memcpy(&c, buffer, sizeof(BufferedChunk));
	c.CopyTo(this);

#if _SERVER
	Data = std::make_unique<u32[]>(Width * Height);
#else
	Data = std::make_unique<FColor8[]>(Width * Height);
#endif
	memcpy(Data.get(), buffer + sizeof(BufferedChunk), Width * Height * 4);

	K_ASSERT(size == GetSize(), "image chunk size mismatch");
}

void KWadChunk_Image::BufferedChunk::CopyFrom(KWadChunk_Image* chunk)
{
	Width = chunk->Width;
	Height = chunk->Height;
	DisplayWidth = chunk->DisplayHeight;
	DisplayHeight = chunk->DisplayHeight;
	PadMethod = chunk->PadMethod;
	bHasTransparency = chunk->bHasTransparency;
}

void KWadChunk_Image::BufferedChunk::CopyTo(KWadChunk_Image* chunk)
{
	chunk->Width = Width;
	chunk->Height = Height;
	chunk->DisplayWidth = DisplayHeight;
	chunk->DisplayHeight = DisplayHeight;
	chunk->PadMethod = PadMethod;
	chunk->bHasTransparency = bHasTransparency;
}
