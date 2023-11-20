#pragma once

#include "kfglobal.h"
#include "grid_cell.h"
#include "cell_coordinate.h"

class KCellMap
{
	const u16 Dimension = 256;
	THashMap<u64, KGridCell> Map;

public:

	KCellMap() = default;
	KCellMap(u16 dimension) : Dimension(dimension) {}

	u16 GetCellDimension() const { return Dimension; }

	KCellCoordinate CoordinateFromPoint(const GVec3& point) const;
	void GetCoordsFromBounds(const GBoundingBox& bounds, TVector<KCellCoordinate>& coords);
	void GetCoordsFromBounds(const GBoundingBox& bounds, u64* coords, u8& count);
	
	void ClearCellData();

	KGridCell* GetCell(u64 coord);
	KGridCell* GetCell(KCellCoordinate coord);
	KGridCell* AddCell(u64 coord);

	void AddCompEntityFromBounds(KEntity* ent, const GBoundingBox& bounds);

	void GetPlanesFromCoord(KCellCoordinate coord, GPlane* planes); // planes should be a size of 6
};