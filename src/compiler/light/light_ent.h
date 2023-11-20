#pragma once

#include "engine/math/polygon.h"

class KLightEntity
{
	friend class KBrushFace;

protected:

	DVec3 Color = DVec3(1, 1, 1);
	DVec3 GIScale = DVec3(1, 1, 1);
	DVec3 Position;
	bool bCastShadows = true;

public:

	bool CastsShadows() { return bCastShadows; }
	const DVec3& GetColor() { return Color; }
	const DVec3& GetGIScale() { return GIScale; }
	
	virtual bool PolyInRange(const DPolygon& poly) const = 0;
	virtual bool PointInRange(const DVec3& point) const = 0;
	virtual bool PolyBoundsInLightBouds(const DPolygon& poly) const = 0;
	virtual bool CanHitPlane(const DPlane& plane) const = 0;
	virtual DVec3 FromPoint(const DVec3& point) const = 0;
	virtual f64 Strength(f64 diffuseFactor, f64 dist) const = 0;
	virtual DVec3 GetPosition(DVec3 point = DVec3(0, 0, 0)) const = 0;

	void ParseCommonLightProperies();
};
