#pragma once

#include "engine/math/aabb.h"
#include "engine/math/polygon.h"

class KPrecipitationVolume
{
	enum
	{
		Rain = 0, Snow = 1, Hail = 2
	};

public:

	GBoundingBox Bounds;
	u8 PrecipitationType = 0;
	GFlt Density;
	GFlt Speed;
	GFlt Spacing;
	GFlt Sway;
	u32 ID = 0;

	TVector<TVector<GVec3>> SpawnPoints, StopPoints;

public:

	~KPrecipitationVolume();
	
	void SetupVolume(const TVector<GPolygon>& polygons, u32 id);

	u8 GetPrecipitationType() const { return PrecipitationType; }
	GFlt GetDensity() const { return Density; }
	GFlt GetSpeed() const { return Speed; }
	GFlt GetSpacing() const { return Spacing; }
	GFlt GetSway() const { return Sway; }
	u32 GetID() const { return ID; }
	GVec3 GetBoundsMin() const { return Bounds.Min; }
	GVec3 GetBoundsMax() const { return Bounds.Max; }
	void GetSpawnPoints(TVector<TVector<GVec3>>& points) const { points = SpawnPoints; }
	void GetStopPoints(TVector<TVector<GVec3>>& points) const { points = StopPoints; }
};