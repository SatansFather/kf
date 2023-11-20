#include "kwad_file.h"
#include "kwad_chunk_image.h"
#include "../render/surface2d.h"
#include <fstream>
#include "kwad_chunk_sound.h"
#include "../global/paths.h"
#include "kwad_chunk_image_hud.h"
#include "zlib.h"
#include "kwad_chunk_obj.h"
#include "kwad_chunk_map.h"
#include "engine/utility/file_utility.h"

#if _WIN32
#include "engine/os/windows/gdi.h"
#endif

#include "../game_instance.h"
#include "kwad_chunk_font.h"

#define COMPRESS_WAD 0

extern KString CCOM_ListWadContents(const KString& val)
{
	// TODO val should eventually be the file followed by filter
	// for now its just filter for the main wad file

	u32 type = KWadFile::WadFileTypeFromString(val);
#if !_DEV
	KGameInstance::Get().MainWadFile.ListContents(type);
#endif

	return "";
}

UPtr<KWadChunk> KWadFile::CreateBlankChunk(u32 type)
{
	switch (type)
	{
		case EWadFileChunkType::Image:
			return std::make_unique<KWadChunk_Image>();
		case EWadFileChunkType::ImageHUD:
			return std::make_unique<KWadChunk_ImageHud>();
		case EWadFileChunkType::Sound:
			return std::make_unique<KWadChunk_Sound>();
		case EWadFileChunkType::Obj:
			return std::make_unique<KWadChunk_Obj>();
		case EWadFileChunkType::Map:
			return std::make_unique<KWadChunk_Map>();
		case EWadFileChunkType::Font:
			return std::make_unique<KWadChunk_Font>();
	}
	return nullptr;
}

KString KWadFile::StringFromWadChunkType(u32 type)
{
	switch (type)
	{
		case EWadFileChunkType::Image:
			return "image";
		case EWadFileChunkType::ImageHUD:
			return "imagehud";
		case EWadFileChunkType::Sound:
			return "sound";
		case EWadFileChunkType::Obj:
			return "obj";
		case EWadFileChunkType::Map:
			return "map";
		case EWadFileChunkType::Font:
			return "font";
	}

	return "";
}

u32 KWadFile::WadFileTypeFromString(const KString& string)
{
	KString lower = string.ToLower();
	if (lower == "image") return EWadFileChunkType::Image;
	if (lower == "imagehud") return EWadFileChunkType::ImageHUD;
	if (lower == "sound") return EWadFileChunkType::Sound;
	if (lower == "obj") return EWadFileChunkType::Obj;
	if (lower == "map") return EWadFileChunkType::Map;
	if (lower == "font") return EWadFileChunkType::Font;
	return EWadFileChunkType::NumChunkTypes;
}

void KWadFile::ListContents(u32 fileType /*= EWadFileChunkType::NumChunkTypes*/)
{
	LOG("Contents of kfdata.kwad");
	for (const auto& kv1 : IndexedData)
	{
		if (fileType < EWadFileChunkType::NumChunkTypes)
		  if (fileType != kv1.first)
			continue;

		for (const auto& kv : kv1.second)
		{
			KString asset = StringFromWadChunkType(kv1.first);
			asset += " - " + kv.first;
			asset += " - " + KString(kv.second.Data.get() ? "loaded" : "not loaded");
			LOG(asset);
		}	
	}
}

#if _PACK
void KWadFile::AddImage(class KSurface2D* surface)
{
	UPtr<KWadChunk_Image> chunk = std::make_unique<KWadChunk_Image>();
	chunk->CreateFromSurface(surface);
	CollectedFiles.push_back(std::move(chunk));
}

void KWadFile::AddSound(const KString& path)
{
	TVector<char> buffer = GetFileData(KString(SOUND_DIR) + path + ".wav", true);
	if (buffer.size() == 0) return;

	UPtr<KWadChunk_Sound> chunk = std::make_unique<KWadChunk_Sound>();
	chunk->Name = path;
	chunk->FileType = EWadFileChunkType::Sound;
	chunk->Data.resize(buffer.size());
	memcpy(chunk->Data.data(), buffer.data(), buffer.size());

	CollectedFiles.push_back(std::move(chunk));
}

void KWadFile::AddObjModel(const KString& fileName)
{
	// open obj and mtl files as text
	TVector<char> obj = GetFileData(RES_DIR + "model/" + fileName + ".obj");
	TVector<char> mtl = GetFileData(RES_DIR + "model/" + fileName + ".mtl");
	if (obj.size() == 0 || mtl.size() == 0) return;

	UPtr<KWadChunk_Obj> chunk = std::make_unique<KWadChunk_Obj>();
	chunk->Name = fileName;
	chunk->FileType = EWadFileChunkType::Obj;
	chunk->ObjSize = obj.size();
	chunk->MtlSize = mtl.size();
	
	chunk->Obj.GetMut().resize(chunk->ObjSize, '\0');
	memcpy((void*)(chunk->Obj.Get().data()), obj.data(), chunk->ObjSize);

	chunk->Mtl.GetMut().resize(chunk->MtlSize, '\0');
	memcpy((void*)(chunk->Mtl.Get().data()), mtl.data(), chunk->MtlSize);

	CollectedFiles.push_back(std::move(chunk));
}

void KWadFile::AddMap(const KString& mapFile)
{
	TVector<char> buffer = GetFileData(MAP_DIR + mapFile + ".kmap", true);
	if (buffer.size() == 0) return;

	UPtr<KWadChunk_Map> chunk = std::make_unique<KWadChunk_Map>();
	chunk->Name = mapFile;
	chunk->FileType = EWadFileChunkType::Map;

	chunk->Data.resize(buffer.size());
	memcpy(chunk->Data.data(), buffer.data(), buffer.size());

	CollectedFiles.push_back(std::move(chunk));
}

void KWadFile::AddFont(const KString& fontFile)
{
	TVector<char> buffer = GetFileData(FONT_DIR + fontFile, true);
	if (buffer.size() == 0) return;
	buffer = GetFileData(FONT_DIR + fontFile, true);
	if (buffer.size() == 0) return;

	UPtr<KWadChunk_Font> chunk = std::make_unique<KWadChunk_Font>();
	chunk->Name = fontFile;
	chunk->FileType = EWadFileChunkType::Font;

	chunk->Data.resize(buffer.size());
	memcpy(chunk->Data.data(), buffer.data(), buffer.size());

	CollectedFiles.push_back(std::move(chunk));
}

#if _WIN32
void KWadFile::AddImageHUD(Gdiplus::Bitmap& bmp, const KString& name)
{
	UPtr<KWadChunk_ImageHud> chunk = std::make_unique<KWadChunk_ImageHud>();
	
	chunk->Name = name;
	chunk->FileType = EWadFileChunkType::ImageHUD;
	chunk->Width = bmp.GetWidth();
	chunk->Height = bmp.GetHeight();

	const u32 colSize = sizeof(Gdiplus::Color);
	chunk->Data.resize(chunk->Width * chunk->Height * colSize);
	
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect r;
	r.X = 0;
	r.Y = 0;
	r.Width = bmp.GetWidth();
	r.Height = bmp.GetHeight();
	
	bmp.LockBits(&r, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
	memcpy(chunk->Data.data(), bitmapData.Scan0, chunk->Data.size());
	bmp.UnlockBits(&bitmapData);

	CollectedFiles.push_back(std::move(chunk));
}
#endif // _WIN32
#endif // _PACK

void KWadFile::ReadFileOffset(u32 offset, u32 size, TVector<char>& buffer)
{
	std::ifstream file(FileName.Get() + ".kwad", std::ios::in | std::ios::ate | std::ios::binary);
	if (!file.is_open()) return;

	buffer.clear();
	buffer.resize(size);
	file.seekg(offset);
	file.read(buffer.data(), size);
}

void KWadFile::WriteFile(const KString& fileName)
{
	// file format:
	// first bytes at start are u32 sizes for each section, can be looked up by EWadFileChunkType * 4
	// each section is 
	//	(count(4) + 
	//		(namesize(2) + 
	//		name(namesize) + 
	//		chunksize(4) + 
	//		data(chunksize)) * 
	//			count)

	u32 allocSize = EWadFileChunkType::NumChunkTypes * sizeof(u32); // byte offsets for each section
	allocSize *= 2; // each section has a count same size as the offset

	TMap<u32, Section> chunks;
	
	for (UPtr<KWadChunk>& chunk : CollectedFiles)
	{
		u32 size = 0;
		size += 2;					// name size
		size += chunk->Name.Size();	// name data
		size += 4;					// chunk size
		size += chunk->GetSize();	// file data

		LOG(chunk->Name + " " + KString(chunk->GetSize() / 1024.f / 1024.f));

		Section& section = chunks[chunk->FileType];

		PackedChunk packed;
		packed.Chunk = chunk.get();
		packed.Name = chunk->Name;
		packed.NameSize = chunk->Name.Size();
		packed.DataSize = chunk->GetSize();

		section.Data.push_back(packed);
		section.ChunkCount++;

		allocSize += size;
		section.WriteSize += size;
	}

	Data = std::make_unique<u8[]>(allocSize);
	u32 offset = EWadFileChunkType::NumChunkTypes * 4;

	// write section offsets
	for (u32 i = 0; i < EWadFileChunkType::NumChunkTypes; i++)
	{
		memcpy(Data.get() + i * 4, &offset, 4);

		u32 writeSize = chunks[i].WriteSize;

		// write size will be 0 if there are no assets of this type
		// make room for a count
		if (writeSize == 0) writeSize = 4;

		offset += writeSize;
	}
	
	// write section data
	for (u32 i = 0; i < EWadFileChunkType::NumChunkTypes; i++)
	{
		const Section& section = chunks[i];
		u32 sectionOffset = *(u32*)(Data.get() + i * 4);
		
		u32 count = section.Data.size();
		memcpy(Data.get() + sectionOffset, &count, 4);
		sectionOffset += 4;

		for (const PackedChunk& c : section.Data)
		{
			memcpy(Data.get() + sectionOffset, &c.NameSize, 2);
			sectionOffset += 2;

			memcpy(Data.get() + sectionOffset, c.Name.CStr(), c.NameSize);
			sectionOffset += c.NameSize;

			memcpy(Data.get() + sectionOffset, &c.DataSize, 4);
			sectionOffset += 4;
			
			c.Chunk->WriteToBuffer(Data.get() + sectionOffset);
			sectionOffset += c.Chunk->GetSize();
		}
	}
	std::ofstream file(KString(fileName + ".kwad").Get(), std::ios::out | std::ios::binary);

#if COMPRESS_WAD
	
	// dont compress initial offsets
	u32 offsetEnd = EWadFileChunkType::NumChunkTypes * 4;

	// map section wont be compressed here because theyre already compressed
	// decompression will crash if we try to compress them with the rest of the data
	u32 sizeBeforeMaps;
	memcpy(&sizeBeforeMaps, Data.get() + EWadFileChunkType::Map * 4, 4);

	uLongf destLen = allocSize * 1.5 + 12;
	UPtr<u8[]> compressedData = std::make_unique<u8[]>(destLen);
	K_ASSERT(compress2(
		compressedData.get(), 
		&destLen, 
		Data.get() + offsetEnd, 
		sizeBeforeMaps - offsetEnd, 
		Z_BEST_COMPRESSION) 
	== Z_OK, "could not compress data");
	
	// write offsets
	file.write((char*)Data.get(), offsetEnd);

	// write the compressed buffer size
	file.write((char*)&destLen, 4);
	
	// write the decompressed size
	file.write((char*)&sizeBeforeMaps, 4);

	// write compressed data
	file.write((char*)compressedData.get(), destLen);	

	// write remaining data
	file.write((char*)Data.get() + sizeBeforeMaps, allocSize - sizeBeforeMaps);	

#else
	file.write((char*)Data.get(), allocSize);	
#endif
}

void KWadFile::LoadWadFile(const KString& file)
{
	FileName = file;

#if COMPRESS_WAD

	TVector<char> buffer;
	{
		TVector<char> fileBuffer = GetFileData(file + ".kwad", true);

		// find sizes
		u32 offsetSize = EWadFileChunkType::NumChunkTypes * 4;
		u32 compSize;
		u32 decompSize;
		memcpy(&compSize, fileBuffer.data() + offsetSize, 4);
		memcpy(&decompSize, fileBuffer.data() + 4 + offsetSize, 4);

		// decompress
		TVector<char> decompBuffer(decompSize);
		uLongf sz = decompSize;
		int ret = uncompress((u8*)decompBuffer.data(), &sz, (u8*)fileBuffer.data() + 8 * offsetSize, compSize);
		K_ASSERT(ret == Z_OK, "could not decompress data: " + KString(ret));

		// create a normal file buffer
		u32 remainingSize = fileBuffer.size() - compSize - offsetSize - 8; // 8 for comp/decomp sizes
		buffer.resize(offsetSize + decompSize + remainingSize);

		// create a regular file structure for reading data
		memcpy(buffer.data(), fileBuffer.data(), offsetSize);
		memcpy(buffer.data() + offsetSize, decompBuffer.data(), decompSize);
		memcpy(buffer.data() + offsetSize + decompSize, fileBuffer.data() + offsetSize + 8 + compSize, remainingSize);

		// fileBuffer and decompBuffer will go out of scope and free some memory
	}
#else
	TVector<char> buffer = GetFileData(file + ".kwad", true);
#endif

	for (u32 i = 0; i < EWadFileChunkType::NumChunkTypes; i++)
	{
#if _SERVER
		if (i != EWadFileChunkType::Map) continue;
#endif
		u32 sectionOffset = *(u32*)(buffer.data() + i * 4);

		u32 sectionCount = *(u32*)(buffer.data() + sectionOffset);
		sectionOffset += 4;

		for (sectionCount; sectionCount > 0; sectionCount--)
		{
			UPtr<KWadChunk> chunk = CreateBlankChunk(i);

			u16 nameSize = *(u16*)(buffer.data() + sectionOffset);
			sectionOffset += 2;

			chunk->Name.GetMut().resize(nameSize, '\0');
			memcpy((void*)(chunk->Name.Get().data()), buffer.data() + sectionOffset, nameSize);
			sectionOffset += nameSize;

			u32 chunkSize = *(u32*)(buffer.data() + sectionOffset);
			sectionOffset += 4;

			//if (i == EWadFileChunkType::Font)
			//	chunk->ReadFromBuffer((u8*)buffer.data() + sectionOffset, chunkSize);

			IndexedData[i][chunk->Name].Offset = sectionOffset;
			IndexedData[i][chunk->Name].Size = chunkSize;
			IndexedData[i][chunk->Name].FileType = i;

			//if (i == EWadFileChunkType::Font)
			//	IndexedData[i][chunk->Name].Data = std::move(chunk);

			sectionOffset += chunkSize;
		}
	}
}
