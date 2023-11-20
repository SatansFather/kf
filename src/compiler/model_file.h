#pragma once

#if 0

#include "kfglobal.h"
#include "../engine/render/vertex_types.h"

class KModelFile
{
public:

	struct DrawCall 
	{
		u32 IndexCount;
		u32 NameSize;
		KString MaterialName;
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
		u32 VerticesOffset, VerticesSize;
		u32 IndicesOffset, IndicesSize;
	} Header;

#if !_SERVER
	// render
	TVector<KWorldVertex> Vertices;
	TVector<u32> Indices;
	TVector<DrawCall> DrawCalls;

	class KSurface2D* TextureAtlas = nullptr;
#endif

private:

	// store this so map file format can be updated
	// new header data should be added to the end
	u32 HeaderSize = 0;

	UPtr<u8[]> UncompressedBuffer;
	UPtr<u8[]> CompressedBuffer;

public:

	~KModelFile();


#if _COMPILER
	void WriteFile(const KString& fileName);

private:

	void BufferPack(u32 offset, const void* data, u32 size);
	void PackTexture(class KSurface2D* surface, u32 offset, u32 size);

	void PrepareHeader();

	void PackMapName();
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

#if !_SERVER
	UPtr<class KSurface2D> ReadTexture(u32 offset, u32 size);
	void ReadDrawCalls(TVector<DrawCall>& calls, u32 offset, u32 size);
#endif

	void DecompressData();
	void ReadMapName();
#if !_SERVER
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
#endif
	void ReadEntities();
	void ReadCollision();
#endif
};

#endif