#pragma once


#include "engine/math/line_segment.h"
#include "engine/math/plane.h"
#include "engine/collision/hit_result.h"
#include "../../engine/math/aabb.h"

struct KPlaneFaceData
{
	TVector<UPtr<class KBrushFace>> Faces;
	DPlane Plane;
	bool bIsGridPlane = false;
};

typedef std::array<u8, sizeof(f64) * 4> KHashedPlane;
typedef TMap<KHashedPlane, KPlaneFaceData> KHashedPlaneMap;

class KBspTree
{
	friend class KMapCompiler;

	UPtr<class KBspNode> HeadNode;

	u32 MinDepth = MAX_U32;
	u32 MaxDepth = 0;
	u32 TotalDepth = 0;
	u32 LeafCount = 0;

	TVector<UPtr<class KLeafPortal>> Portals;

	TVector<UPtr<class KMapSurface>> Surfaces;

	bool bOptimal = false;
	
	u32 TotalFaces = 0;
	u32 LeafedFaces = 0;

public:

	KBspTree(TVector<UPtr<class KBrushFace>>& faces, bool optimal);
	~KBspTree();

	class KBspNode* NodeFromPoint(const DVec3& point) const;
	void FloodFill();


	bool TraceLine(DLineSegment line, DHitResult* hit = nullptr);
	bool TraceLine(FLineSegment line, FHitResult* hit = nullptr);

	bool TraceSphere(DLineSegment line, f64 radius, DHitResult* hit = nullptr);

	KBspNode* GetHead() { return HeadNode.get(); }

	void IncrementFaceCount() { TotalFaces++; }

private:

	void BuildSurfaces(TVector<UPtr<class KBrushFace>>& faces);
	KHashedPlaneMap BuildGridSurfaces(const DBoundingBox& bounds, u32 faceCount);
	void BuildTree();
	void BuildPortals();

	void PartitionSurfaces(TVector<UPtr<class KMapSurface>>* surflist, KBspNode* node, u32 depth);
	class KMapSurface* SelectSplit(TVector<UPtr<class KMapSurface>>& surfaces);

	void GeneratePortals();
	void RemoveInvalidPortals(TVector<UPtr<KLeafPortal>>& ports);

	bool PortalIsValid(class KLeafPortal* portal);

	UPtr<class KLeafPortal> PortalFromPlane(const DPlane& plane);
};