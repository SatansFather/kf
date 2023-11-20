/*
#include "kwad_reader.h"
#include "kwad_file.h"
#include "kwad_chunk_image.h"
#include "kwad_chunk_image_hud.h"
#include "kwad_chunk_sound.h"
#include "kwad_chunk_obj.h"
#include "kwad_chunk_map.h"
#include "../game_instance.h"

template <typename T>
KWadReader<T>::KWadReader(const KString& wad, const KString& name, bool freeOnDestruct / *= true* /)
	:  WadName(wad), ChunkName(name), bFreeOnDestruct(freeOnDestruct)
{
#if !_DEV
	if constexpr (std::is_same<T, KWadChunk_Image>::value)
		FileType = EWadFileChunkType::Image;
	if constexpr (std::is_same<T, KWadChunk_ImageHud>::value)
		FileType = EWadFileChunkType::ImageHUD;
	if constexpr (std::is_same<T, KWadChunk_Sound>::value)
		FileType = EWadFileChunkType::Sound;
	if constexpr (std::is_same<T, KWadChunk_Obj>::value)
		FileType = EWadFileChunkType::Obj;
	if constexpr (std::is_same<T, KWadChunk_Map>::value)
		FileType = EWadFileChunkType::Map;

	K_ASSERT(FileType != MAX_U32, "no file type found");
	
	if (KWadFile* file = GetWadFile())
	{
		auto& map = file->IndexedData[FileType];
		if (map.contains(name))
		{
			KWadFile::IndexedChunk& chunk = map[ChunkName];
			if (!chunk.Data.get())
			{
				TVector<char> buffer;
				file->ReadFileOffset(chunk.Offset, chunk.Size, buffer);
				if (buffer.size() > 0)
				{
					UPtr<KWadChunk> c = file->CreateBlankChunk(chunk.FileType);
					c->ReadFromBuffer((u8*)buffer.data(), chunk.Size);
					chunk.Data = std::move(c);
				}
			}
			Chunk = dynamic_cast<T*>(chunk.Data.get());
		}
	}
#endif
}

template <typename T>
KWadReader<T>::~KWadReader()
{
	if (bFreeOnDestruct && Chunk)
	{
		if (KWadFile* file = GetWadFile())
		{
			auto& map = file->IndexedData[FileType];
			KWadFile::IndexedChunk& chunk = map[ChunkName];
			chunk.Data.reset();
		}
	}
}

template <typename T>
KWadFile* KWadReader<T>::GetWadFile()
{
	return &KGameInstance::Get().MainWadFile;
}

template class KWadReader<KWadChunk_Image>;
template class KWadReader<KWadChunk_ImageHud>;
template class KWadReader<KWadChunk_Sound>;
template class KWadReader<KWadChunk_Obj>;
template class KWadReader<KWadChunk_Map>;*/