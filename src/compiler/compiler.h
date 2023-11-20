#pragma once

//#if _COMPILER

#include <fstream>
#include "kfglobal.h"
#include "entity.h"
#include <mutex>
#include "engine/math/aabb.h"
#include "engine/collision/brush.h"

#define COMPILER_SHOW_WINDOW 0

class KMapCompiler
{
//private:
public:
	
	//class KTestBspNode* bsphead = nullptr;
	//UPtr<class KBspNode> BspHead;

	// TEMPORARY
	class KBvhGrid* Grid = nullptr;

	class KBspTree* BspTree = nullptr;

	UPtr<class KRenderInterface> RenderInterface;

	//UPtr<class KBspTree> BspHead;

	class KMapEntity* WorldSpawn = nullptr;
	class KMapEntity* TempSky = nullptr;

	// stores all brush face geometry before some of it gets removed
	TVector<KPendingCollisionBrush> PendingCollision;

	TVector<UPtr<KMapEntity>> ParsedEntities;
	/*TVector<UPtr<KMapBrush>> WorldBrushes;
	TVector<UPtr<KBrushFace>> WorldFaces;

	TVector<KBrushFace> NonJunkFaces;*/

	// textures that are used in the map
	// need to be referenced in the game data file or packed in the map file
	TVector<class KTexture2D*> UsedTextures;

	// all faces that will be part of the lighting build
	TVector<KBrushFace*> LitFaces;
	TMap<i32, TVector<KBrushFace*>> SmoothingGroups;

	TVector<DPlane> OriginalSurfacePlanes;
	TVector<DVec3> LeakPath;
	//TVector<UPtr<class KBrushFace>> Portals;

	TMap<i32, UPtr<class KFaceAttribs>> FaceAttribMap;
	
	TMap<class KString, UPtr<class KSurface2D>> LoadedTextureSurfaces[2];

	DBoundingBox MapBounds;

	// lock when adding a world face, bsp generation adds from multiple threads
	//std::mutex WorldFaceMutex;

	// temporary until map files are properly saved
	TVector<UPtr<class KPrecipitationVolume>> PendingPrecipitation;

	struct FMapSettings
	{
		f64 LightmapResolution = 4;
		bool bSkipFloodFill = false;
		f32 FogColor[4];
		f32 FogStart = 256;
		f32 FogEnd = 1536;
	};
	FMapSettings MapSettings;

	struct FCompilerSettings
	{
		
	};
	FCompilerSettings CompilerSettings;

	// allows unique ptr forward declares
	~KMapCompiler();

private:

	void ParseFile(const class KString& path);

	void CreateBrushFaces();
	//void InitBrushFaces();
	void CombineWorldBrushes();
	void CreateEntities();
	void BuildSmoothingGroups();

	void MergeRenderableEntityBrushes();

	void RemoveJunkFaces();
	void RemoveInsideFaces();

	void MergeTextures();

	void BuildLightmaps();
	void PackLightmaps();

	void BuildSpatialGrid();

	void WriteFile();

	//void BuildSurfaces(TVector<std::unique_ptr<class KMapSurface>>& surfaces);

	// processes map settings from the worldspawn entity
	void ProcessWorldSpawn(KMapEntity* ent);

	KMapCompiler() = default;

public:

	static KMapCompiler& Get();

	void LoadFaceSurface(const class KString& tex);

	void CompileMap(const class KString& path);
	void AddTexture(class KTexture2D* tex);
	void AddLitFace(KBrushFace* face);
	void AddFaceToSmoothingGroup(i32 group, KBrushFace* face);

	const FMapSettings& GetMapSettings() { return MapSettings; }

	void UpdateParseProgress();

	class KMapEntity* GetWorldSpawn() { return WorldSpawn; }
	class KMapEntity* GetTempSky() { return TempSky; }

};

//#endif
