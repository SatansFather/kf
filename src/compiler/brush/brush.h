
#pragma once

#include <fstream>
#include "kfglobal.h"
#include "brush_face.h"
#include "engine/math/aabb.h"

class KMapBrush
{
	friend class KMapCompiler;

//private:
public:

	TVector<UPtr<KBrushFace>> Faces;

	// the planes that were used to build the vertices
	TVector<DPlane> Planes;

	// individual vertices of the brush, not part of a face
	TVector<DVec3> Vertices;

	DVec3 BrushCenter;

	//bool bBoundingInit = false;
	//DVec3 BoundingMin, BoundingMax;
	DBoundingBox Bounds;

	class KMapEntity* OwningEntity = nullptr;

	bool bIsPenetrable = false;
	bool bIsWater = false;

#if _COMPILER

	// these faces are found to be outside of other brushes
	// to replace Faces once all brushes have been split
	TVector<UPtr<KBrushFace>> PendingNewFaces;
#endif

	// water brushes are moved to worldspawn and lose their entity
	TMap<class KString, class KString> OriginalEntityProperties;
	bool bMovedEntity = false;

public:

	KMapBrush();
	~KMapBrush();

	void CreateFaces(bool loadingMap =false);
	bool PointInsideBrush(const DVec3& point);

#if _COMPILER
	const TVector<DVec3>& GetVertices() const { return Vertices; }
	//DVec3 GetBoundingMin() { return BoundingMin; }
	//DVec3 GetBoundingMax() { return BoundingMax; }
	const DBoundingBox& GetBoundingBox() { return Bounds; }

	void ParseBrush(std::ifstream& file);
	void RemoveFacesFromOtherBrushes(const TVector<UPtr<KMapBrush>>& brushSet);
	bool IsPenetrable() const { return bIsPenetrable; }
	bool IsWater() const { return bIsWater; }
	void MergeFaces();

	UPtr<KMapBrush> CopyExtendedBrush(DVec3 extent);

#endif

	//bool BoundingBoxesOverlap(KMapBrush* other, f64 epsilon = Epsilon<f64>()) const;
};