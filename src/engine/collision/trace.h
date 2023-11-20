#pragma once

#include "engine/math/line_segment.h"
#include "hit_result.h"
#include "engine/math/plane.h"
#include "engine/math/aabb.h"
#include "engine/render/color.h"
#include "../utility/function_arg.h"
#include "broadphase/trace_args.h"

struct ETraceFlags
{
	enum 
	{
		GroundTrace = 1,
	};
};

enum class EEntTraceResult
{
	TestHit,
	TestMiss,
	MatchingEntity,
	MatchingID,
	ChannelMismatch,

	// values below this indicate we can test from the frame before the current comp frame
	TEST_PREV_FRAME, 
	
	NoCollision,
	NoBoundsOverlap,
	Inside
};

inline f64 TraceTimeInBroadphase = 0;
inline f64 TraceTimeInBrush = 0;
inline f64 TraceTimeInEnt = 0;
inline f64 TotalTraceTime = 0;

template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
bool TraceLine(const GLineSegment& line, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames = 0);
bool TraceLine(const GLineSegment& line, GHitResult& hit);

template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames = 0);
bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit);

// function returns false if it should continue, true to break the loop
template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
void PerformOnOverlapping(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<Args, Locals, Return>* func, u8 compFrames = 0);

#if _SERVER
bool PointIsInWater(const GVec3& point);
#else
bool PointIsInWater(const GVec3& point, FColor32& outColor);
#endif

struct KPlaneTestResult
{
	enum 
	{
		StartOut = 1,
		EndOut = 2,
		InFront = 4
	};

	GFlt EnterFraction = -1, ExitFraction = 1, Penetration = -999999999;
	i32 BestIndex = -1;
	u8 Flags = 0;

	bool StartsOut() const { return Flags & StartOut; }
	bool EndsOut() const { return Flags & EndOut; }
	bool HasCollision() const { return !(Flags & InFront); }
	GFlt GetPenetration() const { return Penetration == -999999999 ? 0 : abs(Penetration); }
};

template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
struct KTraceBase
{
	friend class KBvhGrid;

protected:

	GHitResult* Hit = nullptr;
	const GLineSegment* Line = nullptr;
	GFlt OutFraction = 1;
	GBoundingBox TraceBounds;
	GFlt SquaredDistance = 0;
	u8 CompFrames;
	KFunctionArg<Args, Locals, Return>* Condition = nullptr;

	void TestNode(class KBvhNode* node);
	void TestBrush(class KCollisionBrush* brush);
	bool TestPlanes(const TVector<GPlane>& planes, KPlaneTestResult& result);
	bool TestPlanes(const GPlane* planes, u32 planeCount, KPlaneTestResult& result);
	bool FillHit(void* object, KPlaneTestResult& result, const GPlane& bestPlane, u32 objectCollisionChannels);

	virtual void GetDistances(GFlt& start, GFlt& end, const GPlane& plane) = 0;
	virtual void GetNodeBounds(class KBvhNode* node, GBoundingBox& bounds) = 0;
	virtual bool CanHitPlane(const GPlane& plane) { return true; }

#if !_COMPILER
	virtual bool InsideEntity(class KEntProp_CollidableBBox* box) = 0;
	EEntTraceResult TestEntity(class KEntProp_CollidableBBox* box, const GVec3& entPos);
#endif
};

template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
struct KLineTrace : public KTraceBase<Args, Locals, Return>
{
	friend class KBvhGrid;
	using KTraceBase<Args, Locals, Return>::Line;
private:
	KLineTrace() = default;
	void GetDistances(GFlt& start, GFlt& end, const GPlane& plane) override;
	void GetNodeBounds(class KBvhNode* node, GBoundingBox& bounds) override;

#if !_COMPILER
	bool InsideEntity(class KEntProp_CollidableBBox* box) override;
#endif
};

template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
struct KBoxTrace : public KTraceBase<Args, Locals, Return>
{
	friend class KBvhGrid;
	using KTraceBase<Args, Locals, Return>::Line;
private:
	KBoxTrace() = default;
	GVec3 Extent;
	void GetDistances(GFlt& start, GFlt& end, const GPlane& plane) override;
	void GetNodeBounds(class KBvhNode* node, GBoundingBox& bounds) override;
#if !_COMPILER
	bool InsideEntity(class KEntProp_CollidableBBox* box) override;
#endif
};
