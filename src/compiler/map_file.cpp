#include "map_file.h"
#include "compiler.h"
#include <fstream>
#include <filesystem>
#include "engine/render/surface2d.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/scene.h"
#include "engine/render/interface/texture2d.h"
#include "engine/collision/broadphase/bvh_grid.h"
#include "engine/collision/brush.h"
#include "engine/global/paths.h"
#include "md5.h"
#include "zlib.h"

#if !_COMPILER
#include "engine/game/ent_creation.h"
#include "engine/game/match.h"
#include "../engine/game_instance.h"
#include "../engine/render/communication.h"
#endif
#include "../engine/kwad/kwad_reader.h"
#include "../engine/utility/file_utility.h"

KMapFile::~KMapFile() {}

#if _COMPILER

void KMapFile::WriteFile(const KString& fileName)
{
	PrepareHeader();

	UncompressedBuffer = std::make_unique<u8[]>(Header.DataSize);

	PackMapName();
	PackFog();
	PackWorldVertices();
	PackWorldIndices();
	PackWorldCalls();
	PackSkyVertices();
	PackSkyIndices();
	PackSkyCalls();
	PackWorldTextureAtlas();
	PackWorldLightmap();
	PackSkyTextureAtlas();
	PackSkyLightmap();
	PackEntities();
	PackCollision();
	PackLeakLines();

	GenerateHash();
	CompressData();
	WriteToDisk(fileName);
}

void KMapFile::BufferPack(u32 offset, const void* data, u32 size)
{
	memcpy(UncompressedBuffer.get() + offset, data, size);
}

void KMapFile::PackTexture(class KSurface2D* surface, u32 offset, u32 size)
{
	if (surface)
	{
		BufferPack(offset, surface->GetData(), size - 10);
		u32 w = surface->GetPaddedWidth();
		u32 h = surface->GetPaddedHeight();
		bool t = surface->HasTransparency();
		u8 p = surface->GetPadMethod();
		BufferPack(offset + size - 10, &w, 4);
		BufferPack(offset + size - 6, &h, 4);
		BufferPack(offset + size - 2, &t, 1);
		BufferPack(offset + size - 1, &p, 1);
	}
}

void KMapFile::PrepareHeader()
{
	HeaderSize = sizeof(Header);

	const auto addToHeader = [&](u32* offsetAddr, u32 sizeInBytes) -> void
	{
		*offsetAddr = Header.DataSize;
		*(offsetAddr + 1) = sizeInBytes;
		Header.DataSize += sizeInBytes;
	};

	addToHeader(&Header.GlobalFogOffset, sizeof(f32) * 6);
	
	addToHeader(&Header.WorldVerticesOffset, WorldVertices.size() * sizeof(KWorldVertex));
	addToHeader(&Header.WorldIndicesOffset, WorldIndices.size() * sizeof(u32));
	u32 worldDrawSize = 0;
	for (DrawCall& draw : WorldDrawCalls)
		worldDrawSize += sizeof(u32) * 2 + draw.NameSize + draw.GetPropertiesSize() + 4;
	addToHeader(&Header.WorldDrawCallsOffset, worldDrawSize);

	addToHeader(&Header.SkyVerticesOffset, SkyVertices.size() * sizeof(KWorldVertex));
	addToHeader(&Header.SkyIndicesOffset, SkyIndices.size() * sizeof(u32));
	u32 skyDrawSize = 0;
	for (DrawCall& draw : SkyDrawCalls)
		skyDrawSize += sizeof(u32) * 2 + draw.NameSize + draw.GetPropertiesSize() + 4;
	addToHeader(&Header.SkyDrawCallsOffset, skyDrawSize);

	u32 texAtlasSize = 0;
	if (WorldTextureAtlas)
		texAtlasSize = WorldTextureAtlas->GetPaddedWidth() * WorldTextureAtlas->GetPaddedHeight() * sizeof(FColor8) + 10;
	addToHeader(&Header.WorldTextureAtlasOffset, texAtlasSize);

	u32 lightmapSize = 0;
	if (WorldLightmap) 
		lightmapSize = WorldLightmap->GetPaddedWidth() * WorldLightmap->GetPaddedHeight() * sizeof(FColor8) + 10;
	addToHeader(&Header.WorldLightmapOffset, lightmapSize);

	texAtlasSize = 0;
	if (SkyTextureAtlas)
		texAtlasSize = SkyTextureAtlas->GetPaddedWidth() * SkyTextureAtlas->GetPaddedHeight() * sizeof(FColor8) + 10;
	addToHeader(&Header.SkyTextureAtlasOffset, texAtlasSize);

	lightmapSize = 0;
	if (SkyLightmap)
		lightmapSize = SkyLightmap->GetPaddedWidth() * SkyLightmap->GetPaddedHeight() * sizeof(FColor8) + 10;
	addToHeader(&Header.SkyLightmapOffset, lightmapSize);

	u32 entSize = 0;
	for (UPtr<KMapEntity>& ent : KMapCompiler::Get().ParsedEntities)
	{
		if (!ent->ShouldBeCompiled()) continue;

		entSize += 4; // ID
		entSize += 4; // property count
		for (const auto& kv : ent->Properties)
		{
			entSize += kv.first.Size() + 4;
			entSize += kv.second.Size() + 4;
		}
		entSize += sizeof(GVec3); // origin f64
		entSize += 8; // angle
		entSize += 4; // brush count
		if (!ent->ShouldCompileBrushes()) continue;
		for (UPtr<KMapBrush>& brush : ent->Brushes)
		{
			u32 planeCount = brush->Planes.size();
			entSize += 4; // plane count
			entSize += sizeof(GPlane) * planeCount;
		}
	}
	addToHeader(&Header.EntityOffset, entSize);
	
	u32 colSize = 0;
	for (UPtr<KCollisionBrush>& brush : KMapCompiler::Get().Grid->CollisionBrushes)
	{
		colSize += 4; // plane count
		colSize += brush->Planes.size() * sizeof(GPlane);
		colSize += sizeof(GBoundingBox);
		colSize += 12; // channels + pass + id
	}
	addToHeader(&Header.CollisionOffset, colSize);

	u32 lineSize = KMapCompiler::Get().LeakPath.size() * sizeof(DVec3);
	addToHeader(&Header.LeakLineOffset, lineSize);
}

void KMapFile::PackMapName()
{
	//BufferPack(Header.NameOffset, MapName.Get().data(), Header.NameSize);

}

void KMapFile::PackFog()
{
	BufferPack(Header.GlobalFogOffset, FogData, sizeof(f32) * 6);
}

void KMapFile::PackWorldVertices()
{
	BufferPack(Header.WorldVerticesOffset, WorldVertices.data(), Header.WorldVerticesSize);
}

void KMapFile::PackWorldIndices()
{
	BufferPack(Header.WorldIndicesOffset, WorldIndices.data(), Header.WorldIndicesSize);
}

void KMapFile::PackWorldCalls()
{
	u32 callOffset = 0;
	for (auto& call : WorldDrawCalls)
	{
		BufferPack(Header.WorldDrawCallsOffset + callOffset, &call, 8);
		callOffset += 8;
		BufferPack(Header.WorldDrawCallsOffset + callOffset, call.MaterialName.Get().data(), call.NameSize);
		callOffset += call.NameSize;

		u32 propSize = call.GetPropertiesSize();
		BufferPack(Header.WorldDrawCallsOffset + callOffset, &propSize, 4);
		callOffset += 4;

		UPtr<u8[]> data = call.PackMaterialData();
		BufferPack(Header.WorldDrawCallsOffset + callOffset, data.get(), propSize);
		callOffset += propSize;
	}
}

void KMapFile::PackSkyVertices()
{
	BufferPack(Header.SkyVerticesOffset, SkyVertices.data(), Header.SkyVerticesSize);
}

void KMapFile::PackSkyIndices()
{
	BufferPack(Header.SkyIndicesOffset, SkyIndices.data(), Header.SkyIndicesSize);
}

void KMapFile::PackSkyCalls()
{
	u32 callOffset = 0;
	for (auto& call : SkyDrawCalls)
	{
		BufferPack(Header.SkyDrawCallsOffset + callOffset, &call, 8);
		callOffset += 8;
		BufferPack(Header.SkyDrawCallsOffset + callOffset, call.MaterialName.Get().data(), call.NameSize);
		callOffset += call.NameSize;

		u32 propSize = call.GetPropertiesSize();
		BufferPack(Header.SkyDrawCallsOffset + callOffset, &propSize, 4);
		callOffset += 4;

		UPtr<u8[]> data = call.PackMaterialData();
		BufferPack(Header.SkyDrawCallsOffset + callOffset, data.get(), propSize);
		callOffset += propSize;
	}
}

void KMapFile::PackWorldTextureAtlas()
{
	PackTexture(WorldTextureAtlas, Header.WorldTextureAtlasOffset, Header.WorldTextureAtlasSize);
}

void KMapFile::PackWorldLightmap()
{
	PackTexture(WorldLightmap, Header.WorldLightmapOffset, Header.WorldLightmapSize);
}

void KMapFile::PackSkyTextureAtlas()
{
	PackTexture(SkyTextureAtlas, Header.SkyTextureAtlasOffset, Header.SkyTextureAtlasSize);
}

void KMapFile::PackSkyLightmap()
{
	PackTexture(SkyLightmap, Header.SkyLightmapOffset, Header.SkyLightmapSize);
}

void KMapFile::PackEntities()
{
	u32 offset = Header.EntityOffset;
	for (UPtr<KMapEntity>& ent : KMapCompiler::Get().ParsedEntities)
	{
		if (!ent->ShouldBeCompiled()) continue;

		u32 id = ent->GetCompiledID();
		BufferPack(offset, &id, 4);
		offset += 4;

		u32 size = ent->Properties.size();
		BufferPack(offset, &size, 4);
		offset += 4;

		for (const auto& kv : ent->Properties)
		{
			size = kv.first.Size();
			BufferPack(offset, &size, 4);
			offset += 4;
			BufferPack(offset, kv.first.Get().data(), size);
			offset += size;

			size = kv.second.Size();
			BufferPack(offset, &size, 4);
			offset += 4;
			BufferPack(offset, kv.second.Get().data(), size);
			offset += size;
		}

		DVec3 origin(ent->OriginX, ent->OriginY, ent->OriginZ);
		BufferPack(offset, &origin, sizeof(DVec3));
		offset += sizeof(DVec3);

		f64 angle = ent->Angle;
		BufferPack(offset, &angle, 8);
		offset += 8;

		bool compileBrushes = ent->ShouldCompileBrushes();

		u32 brushCount = compileBrushes ? ent->Brushes.size() : 0;
		BufferPack(offset, &brushCount, 4);
		offset += 4;
			
		if (compileBrushes)
		{
			for (UPtr<KMapBrush>& brush : ent->Brushes)
			{	
				u32 planeCount = brush->Planes.size();
				BufferPack(offset, &planeCount, 4);
				offset += 4;
				for (const DPlane& plane : brush->Planes)
				{
					BufferPack(offset, &plane, sizeof(DPlane));
					offset += sizeof(DPlane);
				}
			}
		}
	}
}

void KMapFile::PackCollision()
{
	u32 offset = 0;
	for (UPtr<KCollisionBrush>& brush : KMapCompiler::Get().Grid->CollisionBrushes)
	{
		u32 planeCount = brush->Planes.size();
		BufferPack(Header.CollisionOffset + offset, &planeCount, 4);
		offset += 4;

		for (const GPlane& plane : brush->Planes)
		{
			GPlane p = plane.ToType<GFlt>();
			BufferPack(Header.CollisionOffset + offset, &p, sizeof(GPlane));
			offset += sizeof(GPlane);
		}

		GBoundingBox b = brush->Bounds.ToType<GFlt>();
		BufferPack(Header.CollisionOffset + offset, &b, sizeof(GBoundingBox));
		offset += sizeof(GBoundingBox);

		BufferPack(Header.CollisionOffset + offset, &brush->CollisionChannels, 4);
		offset += 4;

		BufferPack(Header.CollisionOffset + offset, &brush->CollisionPass, 4);
		offset += 4;

		BufferPack(Header.CollisionOffset + offset, &brush->EntityID, 4);
		offset += 4;

	}

	K_ASSERT(offset == Header.CollisionSize, "packed collision size did not match header");
}

void KMapFile::PackLeakLines()
{
	u32 offset = 0;
	for (const DVec3& v : KMapCompiler::Get().LeakPath)
	{
		BufferPack(Header.LeakLineOffset + offset, &v, sizeof(DVec3));
		offset += sizeof(DVec3);
	}
}

void KMapFile::CompressData()
{
	uLongf destLen = Header.DataSize * 1.5 + 12;
	CompressedBuffer = std::make_unique<u8[]>(destLen);
	K_ASSERT(compress2(CompressedBuffer.get(), &destLen, UncompressedBuffer.get(), Header.DataSize, Z_BEST_COMPRESSION) == Z_OK,
		"could not compress data");
	Header.CompressedDataSize = destLen;
}

void KMapFile::WriteToDisk(const KString& fileName)
{
#if _DEV
	std::ofstream file(KString(MAP_DIR + fileName).Get() + ".kmap", std::ios::out | std::ios::binary);
#else
#ifdef WIN32
	CreateDirectory(KString("../maps/").ToWideStr().c_str(), NULL);
#endif

	std::ofstream file("../maps/" + fileName.Get() + ".kmap", std::ios::out | std::ios::binary);
#endif
	file.write((char*)DataHash, 16);

	MapNameSize = MapName.Size();
	file.write((char*)&MapNameSize, 4);
	file.write(MapName.Get().data(), MapNameSize);

	file.write((char*)&HeaderSize, 4);
	file.write((char*)&Header, HeaderSize);
	file.write((char*)CompressedBuffer.get(), Header.CompressedDataSize);
	file.close();
}

#endif

void KMapFile::GenerateHash()
{
	MD5Context c;
	md5Init(&c);
	md5Update(&c, UncompressedBuffer.get(), Header.DataSize);
	md5Finalize(&c);
	
	memcpy(DataHash, c.digest, 16);
}

bool KMapFile::HashMatch(u8* a, u8* b)
{
	return memcmp(a, b, 16) == 0;
}

#if !_COMPILER

#if !_SERVER
UPtr<class KSurface2D> KMapFile::ReadTexture(u32 offset, u32 size)
{
	UPtr<KSurface2D> tex = std::make_unique<KSurface2D>();

	u8* data = UncompressedBuffer.get() + offset;

	u32 w, h;
	bool t;
	u8 p;

	w = *(u32*)(data + size - 10);
	h = *(u32*)(data + size - 6);
	t = *(bool*)(data + size - 2);
	p = *(u8*)(data + size - 1);

	//memcpy(&w, (data + size - 10), 4);
	//memcpy(&h, (data + size - 6), 4);
	//memcpy(&t, (data + size - 2), 1);
	//memcpy(&p, (data + size - 1), 1);

	tex->Data = std::make_unique<FColor8[]>(w * h);
	memcpy(tex->Data.get(), data, size - 10);
	tex->bHasTransparency = t;
	tex->PadMethod = (ESurfacePadMethod)p;
	tex->Width = w;
	tex->Height = h;

	return tex;
}

void KMapFile::ReadDrawCalls(TVector<DrawCall>& calls, u32 offset, u32 size)
{
	u8* data = UncompressedBuffer.get() + offset;

	u32 bytesRead = 0;
	while (bytesRead < size)
	{
		DrawCall call;

		call.IndexCount = *(u32*)(data + bytesRead);
		bytesRead += 4;
		call.NameSize = *(u32*)(data + bytesRead);
		bytesRead += 4;
		for (u32 i = 0; i < call.NameSize; i++)
		{
			call.MaterialName += *((char*)(data + bytesRead));
			bytesRead++;
		}

		u32 propSize = *(u32*)(data + bytesRead);
		bytesRead += 4;

		call.RebuildPendingMaterialProperties(data + bytesRead);
		bytesRead += propSize;

		calls.push_back(call);
	}

	K_ASSERT(bytesRead == size, "bytes read did not match expected size - read " + KString(bytesRead) + " but expected " + KString(size));
}
#endif

bool KMapFile::ReadFromDisk(const KString& fileName, bool preview, bool fullFilePath)
{
	KString path = fullFilePath ? fileName : MAP_DIR + fileName + ".kmap";

	if (!std::filesystem::exists(path.Get()))
	{
		// try to load from a wad

		bool inWad = false;
		// TODO iterate wads
		auto& mapWads = KGameInstance::Get().MainWadFile.IndexedData[EWadFileChunkType::Map];
		if (!mapWads.contains(fileName))
		{
			LOG("Unable to locate map: " + path);
			return false;
		}		
		
		KWadReader<KWadChunk_Map> reader("kfdata", fileName);
		KWadChunk_Map* chunk = reader.GetChunk();
		return LoadFromBuffer((u8*)chunk->Data.data(), preview);
	}

	// we have a raw kmap file
	TVector<char> fileData = GetFileData(path);
	return LoadFromBuffer((u8*)fileData.data(), preview);
}

bool KMapFile::LoadFromBuffer(u8* buffer, bool preview)
{
	u32 offset = 0;

	memcpy(DataHash, buffer, 16);
	offset += 16;

	MapNameSize = *(u32*)(buffer + offset);
	offset += 4;

	MapName.GetMut().resize(MapNameSize, '\0');
	memcpy((void*)(MapName.Get().data()), buffer + offset, MapNameSize);
	offset += MapNameSize;

	if (preview)
	{
		//file.close();
		return true;
	}

	HeaderSize = *(u32*)(buffer + offset);
	offset += 4;

	memcpy(&Header, buffer + offset, HeaderSize);
	offset += HeaderSize;

	CompressedBuffer = std::make_unique<u8[]>(Header.CompressedDataSize);
	memcpy(CompressedBuffer.get(), buffer + offset, Header.CompressedDataSize);
	offset += Header.CompressedDataSize;

	const f32 steps = 16;

	DecompressData();
	UpdateMapLoadProgress(3.f / steps);
#if !_SERVER
	ReadFog();
	ReadWorldVertices();
	UpdateMapLoadProgress(4.f / steps);
	ReadWorldIndices();
	UpdateMapLoadProgress(5.f / steps);
	ReadWorldCalls();
	UpdateMapLoadProgress(6.f / steps);
	ReadSkyVertices();
	UpdateMapLoadProgress(7.f / steps);
	ReadSkyIndices();
	UpdateMapLoadProgress(8.f / steps);
	ReadSkyCalls();
	UpdateMapLoadProgress(9.f / steps);
	ReadWorldTextureAtlas();
	UpdateMapLoadProgress(10.f / steps);
	ReadWorldLightmap();
	UpdateMapLoadProgress(11.f / steps);
	ReadSkyTextureAtlas();
	UpdateMapLoadProgress(12.f / steps);
	ReadSkyTextureAtlas();
	UpdateMapLoadProgress(13.f / steps);
	ReadSkyLightmap();
	UpdateMapLoadProgress(14.f / steps);
#endif
	ReadEntities();
	UpdateMapLoadProgress(15.f / steps);
	ReadCollision();
	UpdateMapLoadProgress(16.f / steps);
#if !_SERVER
	ReadLeakPath();
#endif

	//file.close();
	return true;
}

void KMapFile::DecompressData()
{
	UncompressedBuffer = std::make_unique<u8[]>(Header.DataSize);
	uLongf sz = Header.DataSize;
	int ret = uncompress(UncompressedBuffer.get(), &sz, CompressedBuffer.get(), Header.CompressedDataSize);
	K_ASSERT(ret == Z_OK, "could not decompress data: " + KString(ret));
	K_ASSERT(Header.DataSize == sz, "decompressed size was incorrect");

	u8 loadedHash[16];
	memcpy(loadedHash, DataHash, 16);
	GenerateHash();

	K_ASSERT(HashMatch(loadedHash, DataHash), "hash did not match after decompression");
}

void KMapFile::ReadMapName()
{
	//for (u32 i = 0; i < Header.NameSize; i++)
	//	MapName += *((char*)(UncompressedBuffer.get() + Header.NameOffset + i));
}

#if !_SERVER

void KMapFile::ReadFog()
{
	memcpy(FogData, UncompressedBuffer.get() + Header.GlobalFogOffset, Header.GlobalFogSize);
	if (FogData[3] > 0)
	{
		KFogBuffer fog;
		memcpy(&fog.Color, FogData, sizeof(f32) * 4);
		fog.Data.x = FogData[4];
		fog.Data.y = FogData[5];
		TVector<KFogBuffer> f = { fog };
		KGameInstance::Get().PendingFog = f;
	}
}

void KMapFile::ReadWorldVertices()
{
	WorldVertices.resize(Header.WorldVerticesSize / sizeof(KWorldVertex));
	memcpy(WorldVertices.data(), 
		UncompressedBuffer.get() + Header.WorldVerticesOffset,
		Header.WorldVerticesSize);

	//WorldVertices.insert(
	//	WorldVertices.end(), 
	//	UncompressedBuffer.get() + Header.WorldVerticesOffset, 
	//	UncompressedBuffer.get() + Header.WorldVerticesSize);
}

void KMapFile::ReadWorldIndices()
{
	WorldIndices.resize(Header.WorldIndicesSize / sizeof(u32));
	memcpy(WorldIndices.data(),
		UncompressedBuffer.get() + Header.WorldIndicesOffset,
		Header.WorldIndicesSize);

	//WorldIndices.insert(
	//	WorldIndices.end(),
	//	UncompressedBuffer.get() + Header.WorldIndicesOffset,
	//	UncompressedBuffer.get() + Header.WorldIndicesSize);
}

void KMapFile::ReadWorldCalls()
{
	ReadDrawCalls(WorldDrawCalls, Header.WorldDrawCallsOffset, Header.WorldDrawCallsSize);

	// we have all the data to create the scene now
	KRenderScene* scene = GetRenderInterface()->GetWorldScene();

	for (DrawCall& call : WorldDrawCalls)
	{
		KRenderScene::StaticBufferDrawCall c;
		c.IndexCount = call.IndexCount;
		c.Material = GetRenderInterface()->Materials.GetByName(call.MaterialName);
		scene->CreateMaterialBuffer(c.Material, call.PendingMaterialProperties);
		scene->SubmitStaticDrawCall(std::move(c));
	}

	scene->SubmitVertices(WorldVertices, WorldIndices);
}

void KMapFile::ReadSkyVertices()
{
	SkyVertices.resize(Header.SkyVerticesSize / sizeof(KWorldVertex));
	
	memcpy(SkyVertices.data(),
		UncompressedBuffer.get() + Header.SkyVerticesOffset,
		Header.SkyVerticesSize);
	//SkyVertices.insert(
	//	SkyVertices.end(),
	//	UncompressedBuffer.get() + Header.SkyVerticesOffset,
	//	UncompressedBuffer.get() + Header.SkyVerticesSize);
}

void KMapFile::ReadSkyIndices()
{
	SkyIndices.resize(Header.SkyIndicesSize / sizeof(u32));
	memcpy(SkyIndices.data(),
		UncompressedBuffer.get() + Header.SkyIndicesOffset,
		Header.SkyIndicesSize);
	//SkyIndices.insert(
	//	SkyIndices.end(),
	//	UncompressedBuffer.get() + Header.SkyIndicesOffset,
	//	UncompressedBuffer.get() + Header.SkyIndicesSize);
}

void KMapFile::ReadSkyCalls()
{
	ReadDrawCalls(SkyDrawCalls, Header.SkyDrawCallsOffset, Header.SkyDrawCallsSize);

	// we have all the data to create the scene now
	KRenderScene* scene = GetRenderInterface()->GetSkyboxScene();

	for (DrawCall& call : SkyDrawCalls)
	{
		KRenderScene::StaticBufferDrawCall c;
		c.IndexCount = call.IndexCount;
		c.Material = GetRenderInterface()->Materials.GetByName(call.MaterialName);
		//scene->CreateMaterialBuffer(c.Material, call.PendingMaterialProperties);
		scene->SubmitStaticDrawCall(std::move(c));
	}

	scene->SubmitVertices(SkyVertices, SkyIndices);
}

void KMapFile::ReadWorldTextureAtlas()
{
	if (Header.WorldTextureAtlasSize == 0) return;
	UPtr<KSurface2D> tex = ReadTexture(Header.WorldTextureAtlasOffset, Header.WorldTextureAtlasSize);
	KRenderInterface* iface = GetRenderInterface();
	iface->GetWorldScene()->TextureAtlas = iface->CreateTexture2D();
	iface->GetWorldScene()->TextureAtlas->CreateFromSurface(tex);
	iface->GetWorldScene()->TextureAtlas->FreeSurface();
}

void KMapFile::ReadWorldLightmap()
{
	if (Header.WorldLightmapSize == 0) return;
	UPtr<KSurface2D> tex = ReadTexture(Header.WorldLightmapOffset, Header.WorldLightmapSize);
	KRenderInterface* iface = GetRenderInterface();
	iface->GetWorldScene()->LightmapTexture = iface->CreateTexture2D();
	iface->GetWorldScene()->LightmapTexture->CreateFromSurface(tex);
	iface->GetWorldScene()->LightmapTexture->FreeSurface();
}

void KMapFile::ReadSkyTextureAtlas()
{
	if (Header.SkyTextureAtlasSize == 0) return;
	UPtr<KSurface2D> tex = ReadTexture(Header.SkyTextureAtlasOffset, Header.SkyTextureAtlasSize);
	KRenderInterface* iface = GetRenderInterface();
	iface->GetSkyboxScene()->TextureAtlas = iface->CreateTexture2D();
	iface->GetSkyboxScene()->TextureAtlas->CreateFromSurface(tex);
	iface->GetSkyboxScene()->TextureAtlas->FreeSurface();
}

void KMapFile::ReadSkyLightmap()
{
	if (Header.SkyLightmapSize == 0) return;
	UPtr<KSurface2D> tex = ReadTexture(Header.SkyLightmapOffset, Header.SkyLightmapSize);
	KRenderInterface* iface = GetRenderInterface();
	iface->GetSkyboxScene()->LightmapTexture = iface->CreateTexture2D();
	iface->GetSkyboxScene()->LightmapTexture->CreateFromSurface(tex);
	iface->GetSkyboxScene()->LightmapTexture->FreeSurface();
}

void KMapFile::ReadLeakPath()
{
	if (Header.LeakLineSize == 0) return;
	KRenderInterface* iface = GetRenderInterface();

	iface->GetWorldScene()->LeakPath.resize(Header.LeakLineSize / sizeof(DVec3));

	memcpy(iface->GetWorldScene()->LeakPath.data(),
		UncompressedBuffer.get() + Header.LeakLineOffset,
		Header.LeakLineSize);

	iface->GetWorldScene()->SetupLeakLineVertexBuffer();
}

#endif

void KMapFile::ReadEntities()
{
	u8* data = UncompressedBuffer.get() + Header.EntityOffset;
	
	u32 bytesRead = 0;
	while (bytesRead < Header.EntitySize)
	{
		UPtr<KMapEntity> ent = std::make_unique<KMapEntity>();

		ent->CompiledID = *(u32*)(data + bytesRead);
		bytesRead += 4;

		u32 propCount = *(u32*)(data + bytesRead);
		bytesRead += 4;

		for (u32 i = 0; i < propCount; i++)
		{
			u32 keySize = *(u32*)(data + bytesRead);
			bytesRead += 4;

			KString key;
			for (u32 i = 0; i < keySize; i++)
			{
				key += *((char*)(data + bytesRead));
				bytesRead++;
			}

			u32 valSize = *(u32*)(data + bytesRead);
			bytesRead += 4;

			KString val;
			for (u32 i = 0; i < valSize; i++)
			{
				val += *((char*)(data + bytesRead));
				bytesRead++;
			}

			ent->Properties[key] = val;
			if (key == "classname") ent->Classname = val;
		}

		ent->OriginX = *(f64*)(data + bytesRead);
		bytesRead += 8;

		ent->OriginY = *(f64*)(data + bytesRead);
		bytesRead += 8;

		ent->OriginZ = *(f64*)(data + bytesRead);
		bytesRead += 8;

		// align16 pad
		//bytesRead += 8;

		ent->Angle = *(f64*)(data + bytesRead);
		bytesRead += 8;

		u32 brushCount = *(u32*)(data + bytesRead);
		bytesRead += 4;

		ent->Brushes.reserve(brushCount);

		for (u32 i = 0; i < brushCount; i++)
		{
			UPtr<KMapBrush> brush = std::make_unique<KMapBrush>();
			brush->OwningEntity = ent.get();
			u32 planeCount = *(u32*)(data + bytesRead);
			bytesRead += 4;

			for (u32 p = 0; p < planeCount; p++)
			{
				DPlane plane = *(DPlane*)(data + bytesRead);
				bytesRead += sizeof(DPlane);
				brush->Planes.push_back(plane);
			}
			ent->Brushes.push_back(std::move(brush));
		}

		// create brushes
		if (brushCount > 0)
			ent->CreateBrushFaces(true);

		GetGameMatch()->BaseMapEntities.push_back(std::move(ent));
	}

	GetGameMatch()->ProcessLoadedMapEntities();

	// build collision
	//if (KGameMatch* match = GetGameMatch())
	//	match->BuildPendingCollision();
}

void KMapFile::ReadCollision()
{
	u8* data = UncompressedBuffer.get() + Header.CollisionOffset;
	u32 bytesRead = 0;

	while (bytesRead < Header.CollisionSize)
	{
		UPtr<KCollisionBrush> brush = std::make_unique<KCollisionBrush>();

		u32 planeCount = *(u32*)(data + bytesRead);
		bytesRead += 4;
		brush->Planes.resize(planeCount);

		for (u32 i = 0; i < planeCount; i++)
		{
			brush->Planes[i] = (*(DPlane*)(data + bytesRead)).ToType<GFlt>();
			bytesRead += sizeof(DPlane);
		}

		brush->Bounds = (*(DBoundingBox*)(data + bytesRead)).ToType<GFlt>();
		bytesRead += sizeof(DBoundingBox);
		
		brush->CollisionChannels = *(u32*)(data + bytesRead);
		bytesRead += 4;

		brush->CollisionPass = *(u32*)(data + bytesRead);
		if (brush->CollisionPass & ECollisionMask::Weapon)
			brush->CollisionPass |= ECollisionMask::Rubble;
		bytesRead += 4;

		u32 id = *(u32*)(data + bytesRead);
		bytesRead += 4;

		// compiler doesnt have Object member but never calls this function anyway
		brush->Object = GetGameMatch()->GetEntByID(id);

		if (brush->Object) 
		{
			// TODO this entity can technically have multiple brushes
			brush->Object->MapPosition = brush->Bounds.GetCenter();
			TVector<GPolygon> polys = brush->CalculatePolygons();
			brush->Object->bIsBrushEntity = true;
			brush->Object->MaxBoundsDimension = 
				(brush->Bounds.Max - brush->Bounds.GetCenter()).LengthSq();

			brush->Object->Polygons = polys;
			brush->Object->Planes = brush->Planes;

			brush->Object->OnCollisionBrushAssociation();
		}

		GetGameMatch()->Grid->CollisionBrushes.push_back(std::move(brush));
	}

	GetGameMatch()->Grid->BuildTrees();
}

void KMapFile::UpdateMapLoadProgress(f32 progress)
{
#if !_SERVER
	KRenderBridge::Get().MapLoadProgress = progress;
#endif
}

#endif

u32 KMapFile::DrawCall::GetPropertiesSize()
{
	u32 count = PendingMaterialProperties.size();
	u32 size = 4; // count
	if (count > 0)
	{
		for (auto& m : PendingMaterialProperties)
		{
			size += 4; // property count
			for (const auto& kv : m)
			{
				size += 8; // key and value sizes
				size += kv.first.Size();
				size += kv.second.Size();
			}
		}
	}

	return size;
}

UPtr<u8[]> KMapFile::DrawCall::PackMaterialData()
{
	u32 propSize = GetPropertiesSize();
	UPtr<u8[]> data = std::make_unique<u8[]>(propSize);
	u32 offset = 0;

	const auto copy = [&](void* d, u32 size) -> void
	{
		memcpy(data.get() + offset, d, size);
		offset += size;
	};

	u32 count = PendingMaterialProperties.size();
	copy(&count, 4);

	if (count > 0)
	{
		for (auto& m : PendingMaterialProperties)
		{
			u32 propCount = m.size();
			copy(&propCount, 4);
			for (const auto& kv : m)
			{
				u32 keySize = kv.first.Size();
				u32 valSize = kv.second.Size();
				copy(&keySize, 4);
				copy(&valSize, 4);
				copy((void*)kv.first.Get().data(), keySize);
				copy((void*)kv.second.Get().data(), valSize);
			}
		}
	}

	K_ASSERT(offset == propSize, "size mismatch packing material data - packed " + KString(offset) + " but expected " + KString(propSize));

	return data;
}

void KMapFile::DrawCall::RebuildPendingMaterialProperties(u8* data)
{
	u32 offset = 0;

	const auto read = [&](void* dest, u32 size) -> void
	{
		memcpy(dest, data + offset, size);
		offset += size;
	};

	u32 count;
	read(&count, 4);
	PendingMaterialProperties.resize(count);

	for (u32 i = 0; i < count; i++)
	{
		TMap<KString, KString> propMap;
		u32 propCount = 0;
		read(&propCount, 4);
		for (u32 p = 0; p < propCount; p++)
		{
			u32 keySize, valSize;
			read(&keySize, 4);
			read(&valSize, 4);

			KString key, val;
			key.GetMut().resize(keySize, '\0');
			val.GetMut().resize(valSize, '\0');

			read((void*)key.Get().data(), keySize);
			read((void*)val.Get().data(), valSize);
			
			propMap[key] = val;
		}

		PendingMaterialProperties[i] = propMap;
	}
}
