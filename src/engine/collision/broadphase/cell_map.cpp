#include "cell_map.h"
#include "../../../game/entity/properties/movable.h"
#include "../../../game/entity/properties/collidable.h"

KCellCoordinate KCellMap::CoordinateFromPoint(const GVec3& point) const
{
	// dont allow overflow
	GFlt maxPoint = Dimension * (MAX_I16 - 1);

	GVec3 clamped = point.GetClamped(-maxPoint, maxPoint);
	clamped = // round point to cell resolution
		GVec3(	
			std::floor(clamped.x / Dimension),
			std::floor(clamped.y / Dimension),
			std::floor(clamped.z / Dimension)
		);

	return KCellCoordinate(clamped.x, clamped.y, clamped.z);
}

void KCellMap::GetCoordsFromBounds(const GBoundingBox& bounds, TVector<KCellCoordinate>& coords)
{
	KCellCoordinate mincoord = CoordinateFromPoint(bounds.Min);
	KCellCoordinate maxcoord = CoordinateFromPoint(bounds.Max);

	for (i16 x = mincoord.x; x <= maxcoord.x; x++)
	  for (i16 y = mincoord.y; y <= maxcoord.y; y++)
		for (i16 z = mincoord.z; z <= maxcoord.z; z++)
		  coords.push_back(KCellCoordinate(x, y, z));
}

void KCellMap::GetCoordsFromBounds(const GBoundingBox& bounds, u64* coords, u8& count)
{
	KCellCoordinate mincoord = CoordinateFromPoint(bounds.Min);
	KCellCoordinate maxcoord = CoordinateFromPoint(bounds.Max);

	count = 0;

	for (i16 x = mincoord.x; x <= maxcoord.x; x++)
	{
		for (i16 y = mincoord.y; y <= maxcoord.y; y++)
		{
			for (i16 z = mincoord.z; z <= maxcoord.z; z++)
			{
				coords[count] = KCellCoordinate(x, y, z).ToU64();
				count++;
			}
		}
	}
}

void KCellMap::ClearCellData()
{
	Map.clear();
}

KGridCell* KCellMap::GetCell(u64 coord)
{
	auto it = Map.find(coord);

	if (it != Map.end()) 
		return &(it->second);
	else 
		return nullptr;
}

KGridCell* KCellMap::GetCell(KCellCoordinate coord)
{
	return GetCell(coord.ToU64());
}

KGridCell* KCellMap::AddCell(u64 coord)
{
	return &(Map[coord]);
}

void KCellMap::AddCompEntityFromBounds(KEntity* ent, const GBoundingBox& bounds)
{
#if !_COMPILER
	KCellCoordinate mincoord = CoordinateFromPoint(bounds.Min);
	KCellCoordinate maxcoord = CoordinateFromPoint(bounds.Max);

	KCompEntity comp;
	comp.Entity = ent;
	comp.Position = ent->GetPosition();
	comp.PrevPosition = comp.Position;

	comp.PrevPosition = ent->GetLastFrameNetPosition();
	if (KEntProp_Movable* m = ent->As<KEntProp_Movable>())
	{
		if (m->TeleportedThisFrame())
			comp.Flags |= KCompEntity::Teleported;
	}
	if (KEntProp_CollidableBBox* b = ent->As<KEntProp_CollidableBBox>())
	{
		comp.PositionOffsetZ = b->CrouchDistance / 2;
	}

	for (i16 x = mincoord.x; x <= maxcoord.x; x++)
	  for (i16 y = mincoord.y; y <= maxcoord.y; y++)
	    for (i16 z = mincoord.z; z <= maxcoord.z; z++) 
	      AddCell(KCellCoordinate(x, y, z).ToU64())->AddCompEntity(comp);

#endif
}

void KCellMap::GetPlanesFromCoord(KCellCoordinate coord, GPlane* planes)
{
	GVec3 min(coord.x, coord.y, coord.z);
	min *= Dimension;
	GVec3 max = min + Dimension;
	GBoundingBox bounds(min, max);

	bounds.GetPlanes(planes);
}
