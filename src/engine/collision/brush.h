#pragma once

#include "engine/math/polygon.h"
#include "engine/math/aabb.h"


// the main collision brushes dont need polygon data
struct KPendingCollisionBrush
{
	class KMapBrush* OwningBrush;
	TVector<GPlane> Planes;
	GBoundingBox Bounds;
	u32 CollisionChannels = 0;
	u32 CollisionPass = 0;
	u32 EntityID = 0;
	void* Object = nullptr;
	struct FaceData
	{
		GPlane Plane;
		GPolygon Poly;
	};
	TVector<FaceData> Faces;
	bool HasPlane(const GPlane& plane);
};

class KCollisionBrush
{
public:

	TVector<GPlane> Planes;
	GVec3 Center;
	GBoundingBox Bounds;
	u32 CollisionChannels = 0;
	u32 CollisionPass = 0;
#if _COMPILER
	u32 EntityID = 0;
#else
	class KLoadedMapEntity* Object = nullptr;
#endif
	KCollisionBrush(KPendingCollisionBrush& data);
	KCollisionBrush() = default;

	bool HasPlane(const GPlane& plane, GFlt normEpsilon = .00001, GFlt distEpsilon = .01) const;
	i32 GetPlaneIndex(const GPlane& plane) const;
	GFlt GetPlaneValue(const GPlane& p);

	// order planes based on angle
	// axial comes last, 45 degrees comes first
	// allows us to exit the trace plane check early if entirely in front of a plane
	// steeper angled planes are more likely to allow this early exit
	// trace can never be entirely in front of an axial plane if the bbox was overlapped
	// ^^^ a weird wtf - axial planes at the end cause planes in between brushes to block movement (why?)
	//		so object movement needs to iterate and reverse and skip the early return benefits
	void AddOrderedPlane(const GPlane& plane);

	void AddUniquePlane(const GPlane& plane);

	void AddBevelPlanes(KPendingCollisionBrush& data);

	TVector<GPolygon> CalculatePolygons();
};
