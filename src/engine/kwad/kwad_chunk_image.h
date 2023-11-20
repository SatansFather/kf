#pragma once

#include "kwad_chunk.h"
#include "../render/color.h"

#pragma pack(push, 1)
struct KWadChunk_Image : public KWadChunk
{
	u32 Width = 0;
	u32 Height = 0;
	u32 DisplayWidth = 0;
	u32 DisplayHeight = 0;
	u8 PadMethod;
	bool bHasTransparency = false;

#if _SERVER
	UPtr<u32[]> Data;
#else
	UPtr<FColor8[]> Data;
#endif

	// avoid packing name pointer and vpointer and data pointer in the file
#pragma pack(push, 1)
	struct BufferedChunk
	{
		u32 Width = 0;
		u32 Height = 0;
		u32 DisplayWidth = 0;
		u32 DisplayHeight = 0;
		u8 PadMethod;
		bool bHasTransparency = false;

		void CopyFrom(KWadChunk_Image* chunk);
		void CopyTo(KWadChunk_Image* chunk);
	};
#pragma pack(pop)

#if !_SERVER
	void CreateFromSurface(class KSurface2D* surface);
#endif
	u32 GetSize() override;
	void WriteToBuffer(u8* buffer) override;
	void ReadFromBuffer(u8* buffer, u32 size) override;
};
#pragma pack(pop)
