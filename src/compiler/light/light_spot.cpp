#include "light_spot.h"

UPtr<KLightEntity_Spot> KLightEntity_Spot::Create(class KMapEntity* ent)
{
	UPtr<KLightEntity_Spot> light = std::make_unique<KLightEntity_Spot>();

	return light;
}

bool KLightEntity_Spot::PolyInRange(const DPolygon& poly) const
{
	return poly.Distance(Position) <= Range;
}

bool KLightEntity_Spot::PointInRange(const DVec3& point) const
{
	return point.DistanceSq(GetPosition()) < Range * Range;
}

bool KLightEntity_Spot::PolyBoundsInLightBouds(const DPolygon& poly) const
{
	return true;
}

bool KLightEntity_Spot::CanHitPlane(const DPlane& plane) const
{
	// TODO this
	return false;
}

DVec3 KLightEntity_Spot::FromPoint(const DVec3& point) const
{
	return Position - point;
}

f64 KLightEntity_Spot::Strength(f64 diffuseFactor, f64 dist) const
{
	// TODO this
	return 1;
}

DVec3 KLightEntity_Spot::GetPosition(DVec3 point) const
{
	return Position;
}
