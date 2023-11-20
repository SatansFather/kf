#include "trace.h"
#include "compiler/compiler.h"
#include "broadphase/bvh_grid.h"
#include "engine/game/match.h"
#include "broadphase/bvh_node.h"
#include "../../game/entity/properties/collidable.h"
#include "../../game/entity/entity.h"
#include "broadphase/trace_args.h"

static KBvhGrid* GetCollisionGrid()
{
#if _COMPILER
	return KMapCompiler::Get().Grid;
#else
	return GetGameMatch()->Grid.get();
#endif
}

bool TraceLine(const GLineSegment& line, GHitResult& hit)
{	
	return GetCollisionGrid()->TraceLine(line, hit);
}

template <typename Args, typename Locals, typename Return>
bool TraceLine(const GLineSegment& line, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames)
{
	return GetCollisionGrid()->TraceLine(line, hit, condition, compFrames);
}

bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit)
{
	return GetCollisionGrid()->TraceBox(line, extent, hit);
}

template <typename Args, typename Locals, typename Return>
bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames)
{
	return GetCollisionGrid()->TraceBox(line, extent, hit, condition, compFrames);
}

template <typename Args, typename Locals, typename Return>
void PerformOnOverlapping(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<Args, Locals, Return>* func, u8 compFrames)
{
	static_assert(std::is_same<Args, KEntity*>::value, "function must take KEntity* as parameter");
	
	return GetCollisionGrid()->PerformOnOverlapping(origin, halfExtent,	func, compFrames);
}

bool PointIsInWater(const GVec3& point
#if !_SERVER
, FColor32& outColor
#endif
)
{
	bool inWater = false;
	const auto checkWater = [](KTraceHitParams& hits, KCheckWaterLocalCapture& locals) -> bool
	{
		if (hits.Test->StartsSolid())
		{
			if (hits.Test->HitCollision & ECollisionMask::Water)
				*locals.bInWater = true;

			return false;
		}
		return true;
	};

	KCheckWaterLocalCapture cap;
	cap.bInWater = &inWater;

	KFunctionArg<KTraceHitParams, KCheckWaterLocalCapture, bool> func(checkWater, cap);

	GHitResult hit;
	hit.SearchCollision = ECollisionMask::Water;
	GetCollisionGrid()->TraceLine(GLineSegment(point, point + .05), hit, &func);

#if !_SERVER
	outColor = inWater ? FColor32(.18, .13, .17, .75) : FColor32(0, 0, 0, 0);
#endif

	return inWater;
}

template <typename Args, typename Locals, typename Return>
void KTraceBase<Args, Locals, Return>::TestNode(KBvhNode* node)
{
	if (!node) return;

	GBoundingBox nodebounds;
	GetNodeBounds(node, nodebounds);

	bool test = (SquaredDistance < 10000) ?
		node->GetBounds().Overlaps(TraceBounds) :
		nodebounds.Intersects(*Line);

	if (test)
	{
		if (node->IsLeaf())
		{
			K_ASSERT(node->GetLeafCollision(), "leaf node had no collision brush");
			KTimePoint brushStart = KTime::Now();
			TestBrush(node->GetLeafCollision());
			TraceTimeInBrush += KTime::Since(brushStart);
		}
		else
		{
			TestNode(node->GetA());
			TestNode(node->GetB());
		}
	}
}

template <typename Args, typename Locals, typename Return>
void KTraceBase<Args, Locals, Return>::TestBrush(KCollisionBrush* brush)
{
	if (brush->CollisionPass & Hit->TraceCollision) return;
	if (!(brush->CollisionChannels & Hit->SearchCollision)) return;

	KPlaneTestResult result;
	if (!TestPlanes(brush->Planes, result)) return;
	GPlane p;
	if (result.BestIndex >= 0) p = brush->Planes[result.BestIndex];
	if (FillHit(brush, result, p, brush->CollisionChannels))
	{
		Hit->bHit &= ~GHitResult::BoxEnt;
		Hit->bHit |= GHitResult::WorldBrush;
	}
}

template <typename Args, typename Locals, typename Return>
bool KTraceBase<Args, Locals, Return>::TestPlanes(const TVector<GPlane>& planes, KPlaneTestResult& result)
{
	return TestPlanes(planes.data(), planes.size(), result);
}

template <typename Args, typename Locals, typename Return>
bool KTraceBase<Args, Locals, Return>::TestPlanes(const GPlane* planes, u32 planeCount, KPlaneTestResult& result)
{	
	const GFlt epsilon = Line->a.LengthSq() > (5000 * 5000) ? .125 : .01;
	
	for (i32 i = 0; i < planeCount; i++)
	{
		const GPlane& plane = planes[i];
		GFlt startD, endD;
		GetDistances(startD, endD, plane);

		if (endD > 0) result.Flags |= KPlaneTestResult::EndOut;
		if (startD > 0) result.Flags |= KPlaneTestResult::StartOut;

		// negative endD means line ends behind this plane, thus penetration
		// final penetration is the lowest (greatest negative) end distance of any plane
		if (endD > result.Penetration)
			result.Penetration = endD;
		
		// if both points are in front of a plane on a convex shape, there is no intersection
		if (startD > 0 && (endD >= epsilon || endD >= startD))
		{
			result.Flags |= KPlaneTestResult::InFront;
			return false;
		}

		// if line is entirely behind this plane it can still be clipped by another
		if (startD <= 0 && endD <= 0) continue;

		if (startD > endD)
		{
			// line is entering the brush through this plane
			GFlt fraction = (startD - epsilon) / (startD - endD);
			if (fraction < 0) fraction = 0;

			if (fraction > result.EnterFraction)
			{
				result.EnterFraction = fraction;
				result.BestIndex = i;
			}
		}
		else
		{
			// line is leaving the brush through this plane
			GFlt fraction = (startD + epsilon) / (startD - endD);
			if (fraction > 1) fraction = 1;
			if (fraction < result.ExitFraction) 
				result.ExitFraction = fraction;
		}
	}
	
	return true;
}

#if !_COMPILER
template <typename Args, typename Locals, typename Return>
EEntTraceResult KTraceBase<Args, Locals, Return>::TestEntity(class KEntProp_CollidableBBox* box, const GVec3& entPos)
{
	if (!(box->GetCollisionChannels() & Hit->SearchCollision)) 
		return EEntTraceResult::ChannelMismatch;

	if (box == Hit->BoxEntity) 
		return EEntTraceResult::MatchingEntity;

	if (Hit->BoxEntity && box->MatchingIgnoreID(Hit->BoxEntity->IgnoreID)) 
		return EEntTraceResult::MatchingID;

	GBoundingBox adjustedBounds = box->GetCollisionBounds() + entPos;
	if (!TraceBounds.Overlaps(adjustedBounds)) return EEntTraceResult::NoBoundsOverlap;

	if (Hit->BoxEntity && InsideEntity(box) && Hit->BoxEntity->BlocksChannel(box->GetCollisionChannels())) 
		return EEntTraceResult::Inside;

	GPlane planes[6];
	adjustedBounds.GetPlanes(planes);
	KPlaneTestResult result;
	if (TestPlanes(planes, 6, result))
	{
		GPlane p;
		if (result.BestIndex >= 0) p = planes[result.BestIndex];
		if (FillHit(box, result, p, box->GetCollisionChannels()))
		{
			Hit->bHit &= ~GHitResult::WorldBrush;
			Hit->bHit |= GHitResult::BoxEnt;
			return EEntTraceResult::TestHit;
		}
		
		return EEntTraceResult::TestMiss;
	}

	return EEntTraceResult::NoCollision;
}
#endif

template <typename Args, typename Locals, typename Return>
bool KTraceBase<Args, Locals, Return>::FillHit(void* object, KPlaneTestResult& result, const GPlane& bestPlane, u32 objectCollisionChannels)
{
	if (!result.StartsOut())
	{
		if (Condition)
		{
			// check if we can accept this result
			GHitResult test;
			test.HitCollision = objectCollisionChannels;
			test.SearchCollision = Hit->SearchCollision;
			test.TraceCollision = Hit->TraceCollision;
			test.BoxEntity = Hit->BoxEntity;
			test.Object = object;
			test.bHit |= KHitResult<GFlt>::StartSolid;
			test.Point = Line->a;
			test.PositionOffsetZ = Hit->PositionOffsetZ;
			test.PenetrationDepth = result.GetPenetration();
			if (!result.EndsOut())
			{
				test.bHit |= KHitResult<GFlt>::AllSolid;
				test.Time = 0;
			}

			Args args = { Hit, &test };
			if (Condition->Execute(args))
			{
				OutFraction = 0;
				*Hit = test;
				return true;
			}
			return false;
		}
		else
		{
			Hit->bHit &= ~GHitResult::Block;
			Hit->HitCollision = objectCollisionChannels;
			Hit->Object = object;
			Hit->bHit |= KHitResult<GFlt>::StartSolid;
			Hit->Point = Line->a;
			Hit->PenetrationDepth = result.GetPenetration();
			if (!result.EndsOut())
			{
				Hit->bHit |= KHitResult<GFlt>::AllSolid;
				Hit->Time = 0;
			}
			return true;
		}
		return false;
	}

	if (result.EnterFraction < result.ExitFraction && result.EnterFraction > -1
		&& result.EnterFraction < OutFraction)
	{
		if (result.EnterFraction == 0) result.EnterFraction = 0;
		if (result.EnterFraction < Hit->Time && result.BestIndex >= 0)
		{
			if (Condition)
			{
				// check if we can accept this result
				GHitResult test;
				test.bHit |= GHitResult::Block;
				test.Normal = bestPlane.Normal;
				test.PlaneDistance = bestPlane.D;
				test.Time = result.EnterFraction;
				test.Point = Line->GetPoint(result.EnterFraction);
				test.Object = object;
				test.BoxEntity = Hit->BoxEntity;
				test.HitCollision = objectCollisionChannels;
				test.SearchCollision = Hit->SearchCollision;
				test.TraceCollision = Hit->TraceCollision;
				test.PositionOffsetZ = Hit->PositionOffsetZ;
				test.PenetrationDepth = result.GetPenetration();
				Args args = { Hit, &test };
				if (Condition->Execute(args))
				{
					OutFraction = result.EnterFraction;
					*Hit = test;
					return true;
				}
				return false;
			}
			else
			{
				OutFraction = result.EnterFraction;
				Hit->bHit &= ~GHitResult::AllSolid;
				Hit->bHit &= ~GHitResult::StartSolid;
				Hit->bHit |= GHitResult::Block;
				Hit->Normal = bestPlane.Normal;
				Hit->PlaneDistance = bestPlane.D;
				Hit->Time = OutFraction;
				Hit->Point = Line->GetPoint(OutFraction);
				Hit->Object = object;
				Hit->HitCollision = objectCollisionChannels;
				Hit->PenetrationDepth = result.GetPenetration();
				return true;
			}
		}
	}
	return false;
}

template <typename Args, typename Locals, typename Return>
void KLineTrace<Args, Locals, Return>::GetDistances(GFlt& start, GFlt& end, const GPlane& plane)
{
	start = (Line->a | plane.Normal) - plane.D;
	end = (Line->b | plane.Normal) - plane.D;
}

template <typename Args, typename Locals, typename Return>
void KLineTrace<Args, Locals, Return>::GetNodeBounds(KBvhNode* node, GBoundingBox& bounds)
{
	bounds = node->GetBounds();
}

#if !_COMPILER
template <typename Args, typename Locals, typename Return>
bool KLineTrace<Args, Locals, Return>::InsideEntity(class KEntProp_CollidableBBox* box)
{
	return box->GetCollisionBounds().Contains(Line->a, 0);
}
#endif

template <typename Args, typename Locals, typename Return>
void KBoxTrace<Args, Locals, Return>::GetDistances(GFlt& start, GFlt& end, const GPlane& plane)
{
	GVec3 offset;
	for (u8 i = 0; i < 3; i++)
	{
		if (plane.Normal[i] < 0)
			offset[i] = Extent[i];
		else
			offset[i] = -Extent[i];
	}

	start = plane.SignedDistance(Line->a + offset);
	end = plane.SignedDistance(Line->b + offset);
}

template <typename Args, typename Locals, typename Return>
void KBoxTrace<Args, Locals, Return>::GetNodeBounds(KBvhNode* node, GBoundingBox& bounds)
{
	bounds = node->GetBounds();
	bounds.Min -= Extent;
	bounds.Max += Extent;
}

#if !_COMPILER
template <typename Args, typename Locals, typename Return>
bool KBoxTrace<Args, Locals, Return>::InsideEntity(class KEntProp_CollidableBBox* box)
{
	return box->GetAdjustedBounds().Overlaps(GBoundingBox(Line->a - Extent, Line->a + Extent), 0);
}
#endif

template class KTraceBase<KTraceHitParams, KMoveTraceLocalCapture, bool>;
template class KBoxTrace<KTraceHitParams, KMoveTraceLocalCapture, bool>;
template class KLineTrace<KTraceHitParams, KMoveTraceLocalCapture, bool>;

template class KTraceBase<KTraceHitParams, KCheckWaterLocalCapture, bool>;
template class KBoxTrace<KTraceHitParams, KCheckWaterLocalCapture, bool>;
template class KLineTrace<KTraceHitParams, KCheckWaterLocalCapture, bool>;

template class KTraceBase<KTraceHitParams, KPlaneTestLocalCapture, bool>;
template class KBoxTrace<KTraceHitParams, KPlaneTestLocalCapture, bool>;
template class KLineTrace<KTraceHitParams, KPlaneTestLocalCapture, bool>;

template class KTraceBase<KTraceHitParams, int, bool>;
template class KBoxTrace<KTraceHitParams, int, bool>;
template class KLineTrace<KTraceHitParams, int, bool>;

template class KTraceBase<KTraceHitParams, KCompTraceLocalCapture, bool>;
template class KBoxTrace<KTraceHitParams, KCompTraceLocalCapture, bool>;
template class KLineTrace<KTraceHitParams, KCompTraceLocalCapture, bool>;

template bool TraceBox<KTraceHitParams, KMoveTraceLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KMoveTraceLocalCapture, bool>* condition, u8 flcompFramesags);
template bool TraceLine<KTraceHitParams, KMoveTraceLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KMoveTraceLocalCapture, bool>* condition, u8 compFrames);

template bool TraceBox<KTraceHitParams, KCheckWaterLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KCheckWaterLocalCapture, bool>* condition, u8 compFrames);
template bool TraceLine<KTraceHitParams, KCheckWaterLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KCheckWaterLocalCapture, bool>* condition, u8 compFrames);

template bool TraceBox<KTraceHitParams, KPlaneTestLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KPlaneTestLocalCapture, bool>* condition, u8 compFrames);
template bool TraceLine<KTraceHitParams, KPlaneTestLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KPlaneTestLocalCapture, bool>* condition, u8 compFrames);

// only called from bvh grid itself so we can skip the global trace functions
template bool TraceBox<KTraceHitParams, KCompTraceLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KCompTraceLocalCapture, bool>* condition, u8 compFrames);
template bool TraceLine<KTraceHitParams, KCompTraceLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KCompTraceLocalCapture, bool>* condition, u8 compFrames);

template bool TraceBox<KTraceHitParams, int, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, int, bool>* condition, u8 compFrames);
template bool TraceLine<KTraceHitParams, int, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, int, bool>* condition, u8 compFrames);

template void PerformOnOverlapping<KEntity*, KProjectileExplodeLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KProjectileExplodeLocalCapture, bool>* func, u8 compFrames);
template void PerformOnOverlapping<KEntity*, KTelefragLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KTelefragLocalCapture, bool>* func, u8 compFrames);
template void PerformOnOverlapping<KEntity*, KCheckPickupLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KCheckPickupLocalCapture, bool>* func, u8 compFrames);