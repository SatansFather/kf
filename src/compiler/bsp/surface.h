#pragma once

#include "engine\math\plane.h"
#include "engine\global\types.h"
#include "engine\math\aabb.h"

// a surfaces is basically just a set of faces that share a plane
class KMapSurface
{
public:

	f64 SurfaceArea = 0;

	i32 PlaneIndex = -1;
	bool bIsGridPlane = false;
	//DVec3 BoundingMin, BoundingMax;
	DBoundingBox Bounds;
	DPlane Plane; // planes are always facing POSITIVE for surfaces
	TVector<UPtr<class KBrushFace>> Faces; // both sides of plane
	bool bHasSplit = false; // every surface needs to split eventually

	// portals are created where regular faces dont exist
	// should probably make something other than brush face for this, but brush face already has all the right functions
	//TVector<UPtr<class KBrushFace>> Portals;

	class KBspTree* Tree = nullptr;

	~KMapSurface();

	// updates bounding box and surface area
	void InitNewSurface();	

	void SplitByPlane(const DPlane& plane, bool self, UPtr<KMapSurface>& front, UPtr<KMapSurface>& back);
	void PortalizeSurface();

	UPtr<KMapSurface> CopySurface();
};