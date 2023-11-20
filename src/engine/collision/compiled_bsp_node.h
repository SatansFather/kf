#pragma once

#include "kfglobal.h"
#include "hit_result.h"
#include "engine/math/line_segment.h"
#include "engine/math/plane.h"

class KCompiledBspNode
{
	GPlane SplitPlane;
	u32 LeftOffset = MAX_U32;
	u32 RightOffset = MAX_U32;

public:

	bool IsLeaf() const { return LeftOffset & 0x80000000; }
	bool IsSolidLeaf() const { return RightOffset & 0x80000000; }
	u32 GetLeftOffset() const { return LeftOffset & 0x7FFFFFFF; }
	u32 GetRightOffset() const { return RightOffset & 0x7FFFFFFF; }

	bool TraceLine(const GLineSegment& line, GHitResult& hit);
	bool TraceLine(const GVec3& start, const GVec3& end, GHitResult& hit);

private:

	bool ClipLine(const GVec3& start, const GVec3& end, const GPlane& split, GHitResult* hit);
};