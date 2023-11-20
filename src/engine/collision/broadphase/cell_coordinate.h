#pragma once

#include "engine/global//types.h"

// a cell in a spacial grid
class KCellCoordinate
{
public:

	// index stepped every CellResolution units
	i16 x, y, z;

	KCellCoordinate() = default;
	KCellCoordinate(u64 i);
	KCellCoordinate(i16 x, i16 y, i16 z) : x(x), y(y), z(z) {}

	u64 ToU64() const;
	static KCellCoordinate FromU64(u64 i);

	bool operator==(const KCellCoordinate& other) const
	{
		return x == other.x
			&& y == other.y
			&& z == other.z;
	}

	class KString ToString() const;
};
