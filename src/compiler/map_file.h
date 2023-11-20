#pragma once

#include "kfglobal.h"
#include "../engine/render/vertex_types.h"

class KMapFile
{
public:

	struct DrawCall 
	{
		u32 IndexCount;
		u32 NameSize;
		KString MaterialName;
		TVector<TMap<KString, KString>> PendingMaterialProperties;

		u32 GetPropertiesSize();
		UPtr<u8[]> PackMaterialData();
		void RebuildPendingMaterialProperties(u8* data);
	};

	u8 DataHash[16];
	u32 MapNameSize;
	KString MapName;

	struct
	{
		u32 FileVersion = 0;
		u32 DataSize = 0;
		u32 CompressedDataSize = 0;

		//u32 NameOffset, NameSize;
		u32 GlobalFogOffset, GlobalFogSize;
		u32 WorldVerticesOffset, WorldVerticesSize;
		u32 WorldIndicesOffset, WorldIndicesSize;
		u32 WorldDrawCallsOffset, WorldDrawCallsSize;
		u32 SkyVerticesOffset, SkyVerticesSize;
		u32 SkyIndicesOffset, SkyIndicesSize;
		u32 SkyDrawCallsOffset, SkyDrawCallsSize;
		u32 WorldTextureAtlasOffset, WorldTextureAtlasSize;
		u32 SkyTextureAtlasOffset, SkyTextureAtlasSize;
		u32 WorldLightmapOffset, WorldLightmapSize;
		u32 SkyLightmapOffset, SkyLightmapSize;
		u32 EntityOffset, EntitySize;
		u32 CollisionOffset, CollisionSize;
		u32 LeakLineOffset, LeakLineSize;
	} Header;

#if !_SERVER
	// render
	TVector<KWorldVertex> WorldVertices;
	TVector<u32> WorldIndices;
	TVector<DrawCall> WorldDrawCalls;

	TVector<class KWorldVertex> SkyVertices;
	TVector<u32> SkyIndices;
	TVector<DrawCall> SkyDrawCalls;

	class KSurface2D* WorldTextureAtlas = nullptr;
	class KSurface2D* WorldLightmap = nullptr;

	class KSurface2D* SkyTextureAtlas = nullptr;
	class KSurface2D* SkyLightmap = nullptr;

	f32 FogData[6];
	// 0-3 for color, 4 for start, 5 for end
#endif

	// entity
	TVector<class KMapEntity*> Entities;

private:

	// store this so map file format can be updated
	// new header data should be added to the end
	u32 HeaderSize = 0;

	UPtr<u8[]> UncompressedBuffer;
	UPtr<u8[]> CompressedBuffer;

public:

	~KMapFile();


#if _COMPILER
	void WriteFile(const KString& fileName);

private:

	void BufferPack(u32 offset, const void* data, u32 size);
	void PackTexture(class KSurface2D* surface, u32 offset, u32 size);

	void PrepareHeader();

	void PackMapName();
	void PackFog();
	void PackWorldVertices();
	void PackWorldIndices();
	void PackWorldCalls();
	void PackSkyVertices();
	void PackSkyIndices();
	void PackSkyCalls();
	void PackWorldTextureAtlas();
	void PackWorldLightmap();
	void PackSkyTextureAtlas();
	void PackSkyLightmap();
	void PackEntities();
	void PackCollision();
	void PackLeakLines();
	void CompressData();
	void WriteToDisk(const KString& fileName);
#endif

public:

	void GenerateHash();
	bool HashMatch(u8* a, u8* b);
#if !_COMPILER
	bool ReadFromDisk(const KString& fileName, bool preview = false, bool fullFilePath = false);
#endif

private:

#if !_COMPILER

	bool LoadFromBuffer(u8* buffer, bool preview);

#if !_SERVER
	UPtr<class KSurface2D> ReadTexture(u32 offset, u32 size);
	void ReadDrawCalls(TVector<DrawCall>& calls, u32 offset, u32 size);
#endif

	void DecompressData();
	void ReadMapName();
#if !_SERVER
	void ReadFog();
	void ReadWorldVertices();
	void ReadWorldIndices();
	void ReadWorldCalls();
	void ReadSkyVertices();
	void ReadSkyIndices();
	void ReadSkyCalls();
	void ReadWorldTextureAtlas();
	void ReadWorldLightmap();
	void ReadSkyTextureAtlas();
	void ReadSkyLightmap();
	void ReadLeakPath();
#endif
	void ReadEntities();
	void ReadCollision();
#endif

	void UpdateMapLoadProgress(f32 progress);
};