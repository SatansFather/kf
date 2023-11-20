#pragma once

#include "engine/math/polygon.h"

class KLeafPortal 
{
public:

	TVector<DVec3> Vertices;
	DPlane Plane;
	DVec3 Center;
	f64 SurfaceArea = 0;

	TVector<class KBspNode*> LeafNodes;

	DPolygon CreatePoly();

	// these functions are shared with brush face
	// implementations are almost the same with some key differences, mainly the vertex types
	// would be "cleaner" to just have both classes inherit from something
	// would "take less time" for me to just copy paste and make slight adjustments
	UPtr<KLeafPortal> SplitByPlane(DPlane plane, f64 epsilon = .001);
	void InitFromVertices();
	void SortVerticesCW();
};

