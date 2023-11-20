#include "cell_coordinate.h"
#include "engine/utility/kstring.h"

KCellCoordinate::KCellCoordinate(u64 i)
{
	x = (i & 0x0000FFFF00000000ULL) >> 32ULL;
	y = (i & 0x00000000FFFF0000ULL) >> 16ULL;
	z = (i & 0x000000000000FFFFULL);
}

u64 KCellCoordinate::ToU64() const
{
	u64 out = 0;
	out |= u64(*(u16*)&x) << 32ULL;
	out |= u64(*(u16*)&y) << 16ULL;
	out |= u64(*(u16*)&z);
	return out;
}

KCellCoordinate KCellCoordinate::FromU64(u64 i)
{
	return KCellCoordinate(i);
}

KString KCellCoordinate::ToString() const
{
	return KString(x) + " " + KString(y) + " " + KString(z);
}
