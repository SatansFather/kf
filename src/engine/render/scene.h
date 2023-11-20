#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/math/aabb.h"
#include "vertex_types.h"
#include "bufferslots.h"

// scene data from the map
class KRenderScene
{
	friend class KMapCompiler;	
	friend class KMapFile;
		
public:
	struct StaticBufferDrawCall
	{
		u32 IndexCount = 0, IndexOffset = 0;
		class KMaterial* Material = nullptr;
		UPtr<KGpuBuffer> Buffer;

#if _COMPILER
		TVector<TMap<KString, KString>> PendingMaterialProperties;
#endif
	};

	TVector<StaticBufferDrawCall> OpaqueDrawCalls;
	TVector<StaticBufferDrawCall> TransparentDrawCalls;
	TVector<StaticBufferDrawCall> WaterDrawCalls;

	TVector<KWorldVertex> WorldVertices;
	TVector<u32> WorldIndices;

private:

	TVector<DVec3> LeakPath;
	FBoundingBox SceneBounds;

	// static brush geometry using normal shader
	UPtr<class KVertexBuffer> StaticVertexBuffer;
	u32 StaticVertexCount = 0;
	u32 StaticIndexCount = 0;


	UPtr<class KTexture2D> TextureAtlas;
	UPtr<class KTexture2D> LightmapTexture;

	UPtr<class KVertexBuffer> LeakLineVertexBuffer;
	u32 LeakLineIndexCount = 0;

	TMap<u32, UPtr<struct KPrecipitationBuffer>> Precipitation;

	// used to keep track of index offset while adding draw calls
	u32 DrawCallsIndexOffset = 0;

public:

	KRenderScene();
	~KRenderScene();

	class KVertexBuffer* GetVertexBuffer() { return StaticVertexBuffer.get(); }
	u32 GetIndexCount() const { return StaticIndexCount; }
	u32 GetVertexCount() const { return StaticVertexCount; }
	
#if _COMPILER
	// create StaticVertexBuffer from these faces
	void SubmitFaces(TVector<UPtr<class KBrushFace>>& faces);
#endif

	void SubmitVertices(const TVector<KWorldVertex>& verts, const TVector<u32>& inds);

	void SubmitStaticDrawCall(StaticBufferDrawCall call);

	void DrawOpaque(bool sky = false, class KRenderTarget* target = nullptr);
	void DrawTransparent(bool sky = false);
	void DrawWater(bool sky = false);

	void AddPrecipitation(const class KPrecipitationVolume& p);
	void DrawPrecipitation();

	void LoadLeakLines();
	void DrawLeakLines();

	void SetupLeakLineVertexBuffer();

	void CreateLightmap(const TVector<u32>& colors, u32 w, u32 h);

	const FBoundingBox& GetBounds() const { return SceneBounds; }

	KTexture2D* GetTextureAtlas() { return TextureAtlas.get(); }
	KTexture2D* GetLightmap() { return LightmapTexture.get(); }

	void CreateMaterialBuffer(KMaterial* mat, TVector<TMap<KString, KString>>& m);
	void CreateMaterialBufferForCall(StaticBufferDrawCall& call);
};

#endif