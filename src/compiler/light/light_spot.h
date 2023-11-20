#pragma once

#include "light_ent.h"

class KLightEntity_Spot : public KLightEntity
{
	friend class KBrushFace;

public:

	DVec3 Position;
	f32 Range;
	f32 InnerRadius, OuterRadius;
	f32 Falloff;

	static UPtr<KLightEntity_Spot> Create(class KMapEntity* ent);

	bool PolyInRange(const DPolygon& poly) const override;
	bool PointInRange(const DVec3& point) const override;
	bool PolyBoundsInLightBouds(const DPolygon& poly) const override;
	bool CanHitPlane(const DPlane& plane) const override;
	DVec3 FromPoint(const DVec3& point) const override;
	f64 Strength(f64 diffuseFactor, f64 dist) const override;
	DVec3 GetPosition(DVec3 point = DVec3(0, 0, 0)) const override;
};