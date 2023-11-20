#pragma once

#include "kfglobal.h"
#include "kwad_chunk.h"

#if _WIN32
// forward declare
// cant include gdi in this header because windows sucks and it cant compile because of min/max macros
namespace Gdiplus
{
	class Bitmap;
}
#endif

struct EWadFileChunkType
{
	enum
	{
		Image,
		ImageHUD,
		Sound,
		Obj,
		Font,

		// maps are already compressed and cant be compressed again
		// needs to come last so we can compress everything else up to maps
		Map, 

		NumChunkTypes
	};
};

class KWadFile
{	
public:

	KString FileName;

	// a chunk that has been parsed from the file and may or may not be loaded
	struct IndexedChunk
	{
		u32 Offset = 0;
		u32 Size = 0;
		u32 FileType = EWadFileChunkType::NumChunkTypes;
		UPtr<KWadChunk> Data;
	};

	TMap<u32, TMap<KString, IndexedChunk>> IndexedData;

private:
	// files that are awaiting pack
	TVector<UPtr<KWadChunk>> CollectedFiles;

	UPtr<u8[]> Data;

	struct PackedChunk
	{
		u16 NameSize = 0;
		KString Name;
		u32 DataSize = 0;
		KWadChunk* Chunk = nullptr;
	};

	struct Section
	{
		u32 Type = EWadFileChunkType::NumChunkTypes;
		u32 FileOffset = 0;
		u32 ChunkCount = 0;
		TVector<PackedChunk> Data;

		// not in file, used for determining offsets
		u32 WriteSize = 0;
	};

public:

	UPtr<KWadChunk> CreateBlankChunk(u32 type);

	static KString StringFromWadChunkType(u32 type);
	static u32 WadFileTypeFromString(const KString& string);

	void ListContents(u32 fileType = EWadFileChunkType::NumChunkTypes);

#if _PACK
	void AddImage(class KSurface2D* surface);
	void AddSound(const KString& path);
	void AddObjModel(const KString& fileName);
	void AddMap(const KString& mapFile);
	void AddFont(const KString& fontFile);
#endif

#if _WIN32
	void AddImageHUD(Gdiplus::Bitmap& bmp, const KString& name);
#endif

	// TODO get data functions
	// might want to make a wad reader class with a destructor that frees the memory 
	// should be a template class with the chunk type as T

	void ReadFileOffset(u32 offset, u32 size, TVector<char>& buffer);

	void WriteFile(const KString& fileName);

	void LoadWadFile(const KString& file);
};