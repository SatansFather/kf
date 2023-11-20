#include "compiled_bsp_node.h"

bool KCompiledBspNode::TraceLine(const GLineSegment& line, GHitResult& hit)
{
	return TraceLine(line.a, line.b , hit);
}

bool KCompiledBspNode::TraceLine(const GVec3& start, const GVec3& end, GHitResult& hit)
{
	return ClipLine(start, end, SplitPlane, &hit);
}

bool KCompiledBspNode::ClipLine(const GVec3& start, const GVec3& end, const GPlane& split, GHitResult* hit)
{
#if 0
	if (hit->bHit) return false;
	GLineSegment line(start, end);
	if (IsLeaf())
	{
		if (IsSolidLeaf())
		{
			if (hit)
			{
				hit->bHit = true;
				GFlt hitdist = 1;
				split.Intersects(line, hitdist, 0);
				hit->Normal = ((line.b - line.a) | split.Normal) > 0 ? -split.Normal : split.Normal;
				hit->Point = line.GetPoint(hitdist);
				hit->Time = hitdist;
			}
			return true;
		}
		return false;
	}

	EPointSide sideA = SplitPlane.ClassifyPoint(start, 0);
	EPointSide sideB = SplitPlane.ClassifyPoint(end, 0);

	if (sideA == EPointSide::On)
	{
		if (sideB == sideA)
		{
			sideA = EPointSide::Front;
			sideB = EPointSide::Front;
		}
		else
			sideA = sideB;

	}
	else if (sideB == EPointSide::On)
	{
		if (sideB == sideA)
		{
			sideA = EPointSide::Front;
			sideB = EPointSide::Front;
		}
		else
			sideB = sideA;
	}

	if (sideA == sideB)
	{
		if (sideA == EPointSide::Front)
			return GetFront()->ClipLine(start, end, split, hit);
		else
			return GetBack()->ClipLine(start, end, split, hit);
	}
	else
	{
		GFlt dist = -1;
		if (SplitPlane.Intersects(line, dist, 0))
		{
			GVec3 p = line.GetPoint(dist);

			KCompiledBspNode* nodeA = (sideA == EPointSide::Behind ? GetBack() : GetFront());
			KCompiledBspNode* nodeB = (sideB == EPointSide::Behind ? GetBack() : GetFront());


			bool blocked = nodeA ? nodeA->ClipLine(start, p, SplitPlane, hit) : false;

			if (!blocked)
				return nodeB ? nodeB->ClipLine(p, end, SplitPlane, hit) : false;

			return false;
		}
	}
#endif
	return false;
}
