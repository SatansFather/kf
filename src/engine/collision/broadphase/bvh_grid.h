#pragma once

#include "cell_coordinate.h"
#include "grid_cell.h"
#include "engine/math/aabb.h"
#include "engine/math/line_segment.h"
#include "engine/collision/hit_result.h"
#include "engine/collision/brush.h"
#include "../../utility/function_arg.h"
#include "trace_args.h"
#include "cell_map.h"

class KBvhGrid
{
	// data for all static geometry + trees is allocated in one block 
	u8* Data = nullptr;

	// gets an address in Data from a hashed coordinate
	THashMap<u64, u8*> Table;



public:

	TVector<class KPendingCollisionBrush> PendingCollision;

	TVector<UPtr<class KCollisionBrush>> CollisionBrushes;

	KCellMap StaticGrid;
	KCellMap EntityGrid;

	// objects are responsible for removing themselves from this when they are destroyed
	//THashMap<u64, THashSet<KGenericObjRef>> DynamicObjects;

	KBvhGrid() = default;
	KBvhGrid(u16 dimension, u16 entDimension);

	~KBvhGrid();

	u8* AddressFromPoint(const GVec3& point);
	u8* AddressFromCoordinate(const KCellCoordinate& coord);

	void AddBvh(u64 coord, TVector<class KCollisionBrush*>& brushes);

	template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
	bool TraceLine(const GLineSegment& line, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition = nullptr, u8 compFrames = 0);

	template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
	bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition = nullptr, u8 compFrames = 0);
	//bool TraceCylinder(const GLineSegment& line, flt_t halfHeight, flt_t radius, GHitResult& hit);


	template <typename Args = KTraceHitParams, typename Locals = int, typename Return = bool>
	void PerformOnOverlapping(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<Args, Locals, Return>* func, u8 compFrames);

	void BuildFromPending();
	void BuildTrees();
};
