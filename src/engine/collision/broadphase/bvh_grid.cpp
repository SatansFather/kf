#include "bvh_grid.h"

// DELETE
#include "../../system/terminal/terminal.h"
#include "../../system/time.h"
#include "../../utility/thread_pool.h"
#include "../brush.h"
#include "game/entity/entity.h"
#include "../trace.h"

#if _COMPILER
#include "engine/system/terminal/terminal_progress.h"
#endif
// DELETE
#include "../../../game/entity/properties/collidable.h"
#include "trace_args.h"
#include "../../net/state.h"
//#include "../../net/player.h"

KBvhGrid::KBvhGrid(u16 dimension, u16 entDimension) :
	StaticGrid(KCellMap(dimension)),
	EntityGrid(KCellMap(entDimension))
{}

KBvhGrid::~KBvhGrid() {}

u8* KBvhGrid::AddressFromPoint(const GVec3& point)
{
	return nullptr;
	//return Table[CoordinateFromPoint(point).ToU64()];
}

u8* KBvhGrid::AddressFromCoordinate(const KCellCoordinate& coord)
{
	return nullptr;
	//return Table[coord.ToU64()];
}

void KBvhGrid::AddBvh(u64 coord, TVector<class KCollisionBrush*>& brushes)
{
	StaticGrid.AddCell(coord)->StaticCollision.InitFromBrushes(brushes);
}

template <typename Args, typename Locals, typename Return>
bool KBvhGrid::TraceLine(const GLineSegment& line, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames)
{
	KLineTrace<Args, Locals, Return> trace;
	trace.Condition = condition;
	trace.OutFraction = 1;
	trace.Hit = &hit;
	trace.Line = &line;
	trace.TraceBounds.Update(line.a);
	trace.TraceBounds.Update(line.b);
	trace.SquaredDistance = line.a.DistanceSq(line.b);
	trace.CompFrames = compFrames;

	hit.Reset();
	hit.Point = line.b;

	GBoundingBox linebounds;
	linebounds.Update(line.a);
	linebounds.Update(line.b);
	
	//BvhMap[0].TraceLine(line, hit);
	//return hit.bHit;

	KCellCoordinate mincoord = StaticGrid.CoordinateFromPoint(linebounds.Min);
	KCellCoordinate maxcoord = StaticGrid.CoordinateFromPoint(linebounds.Max);

	GPlane cellPlanes[6];

	if (hit.SearchCollision | 0x0000FFFF)
	{
		/*KCellCoordinate startCoord = StaticGrid.CoordinateFromPoint(line.a);
		KCellCoordinate endCoord = StaticGrid.CoordinateFromPoint(line.b);

		KCellCoordinate coord = startCoord;
		GVec3 currentPoint = line.a;
		


		KGridCell* pendingTouch[8];
		i32 touchCount = 0;

		do
		{
			for (touchCount; touchCount >= 0; touchCount--)
			{
				
			}

			if (KGridCell* cell = StaticGrid.GetCell(coord))
			{
				trace.TestNode(cell->StaticCollision.GetHead());
			}

			if (hit.bHit)
			{
				break;
			}
			else
			{
				// advance coordinate
				StaticGrid.GetPlanesFromCoord(coord, cellPlanes);
			}
		} while (coord != endCoord)*/
		

		for (i16 x = mincoord.x; x <= maxcoord.x; x++)
		{
			for (i16 y = mincoord.y; y <= maxcoord.y; y++)
			{
				for (i16 z = mincoord.z; z <= maxcoord.z; z++)
				{
					u64 coord = KCellCoordinate(x, y, z).ToU64();
					if (KGridCell* cell = StaticGrid.GetCell(coord))
					{
						trace.TestNode(cell->StaticCollision.GetHead());
					}	
				}
			}
		}
	}

#if !_COMPILER
	if (hit.SearchCollision >= ECollisionMask::PlayerCharacter) // player character is the first entity channel
	{		
		KCellMap* cellMap = &EntityGrid;
		if (compFrames > 0)
		{
			if (compFrames > MAX_SNAPSHOTS) compFrames = MAX_SNAPSHOTS;
			u32 frame = KTime::FrameCount() - compFrames;
			cellMap = &(GetNetState()->PositionSnapshots[frame % MAX_SNAPSHOTS]);
		}

		mincoord = cellMap->CoordinateFromPoint(linebounds.Min);
		maxcoord = cellMap->CoordinateFromPoint(linebounds.Max);
		for (i16 x = mincoord.x; x <= maxcoord.x; x++)
		{
			for (i16 y = mincoord.y; y <= maxcoord.y; y++)
			{
				for (i16 z = mincoord.z; z <= maxcoord.z; z++)
				{
					u64 coord = KCellCoordinate(x, y, z).ToU64();
					if (KGridCell* cell = cellMap->GetCell(coord))
					{
						KTimePoint entStart = KTime::Now();

						if (compFrames == 0)
						{
							for (KEntProp_CollidableBBox* box : cell->Entities)
								trace.TestEntity(box, box->GetEntity()->GetPosition());
						}
						else
						{
							for (KCompEntity& ent : cell->CompEntities)
							  if (KEntity* e = ent.Entity.Get())
								trace.TestEntity(e->As<KEntProp_CollidableBBox>(), ent.Position);
						}

						TraceTimeInEnt += KTime::Since(entStart);
					}
				}
			}
		}
	}
#endif

	return hit.bHit;
}

template <typename Args, typename Locals, typename Return>
bool KBvhGrid::TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<Args, Locals, Return>* condition, u8 compFrames)
{
	KTimePoint traceStart = KTime::Now();

	GVec3 adjustedExtent = extent;
	adjustedExtent.z += hit.PositionOffsetZ;

	GLineSegment adjustedLine = line;
	adjustedLine = adjustedLine.AdjustLineZ(hit.PositionOffsetZ);

	KBoxTrace<Args, Locals, Return> trace;
	trace.Condition = condition;
	trace.OutFraction = 1;
	trace.Hit = &hit;
	trace.Line = &adjustedLine;
	trace.TraceBounds.Update(adjustedLine.a);
	trace.TraceBounds.Update(adjustedLine.b);
	trace.Extent = adjustedExtent;
	trace.TraceBounds.Min -= adjustedExtent;
	trace.TraceBounds.Max += adjustedExtent;
	trace.SquaredDistance = adjustedLine.a.DistanceSq(adjustedLine.b);
	trace.CompFrames = compFrames;

	hit.Reset();
	hit.Point = adjustedLine.b;

	GBoundingBox linebounds;
	linebounds.Update(adjustedLine.a);
	linebounds.Update(adjustedLine.b);
	linebounds.Min -= adjustedExtent;
	linebounds.Max += adjustedExtent;

	KCellCoordinate mincoord = StaticGrid.CoordinateFromPoint(linebounds.Min);
	KCellCoordinate maxcoord = StaticGrid.CoordinateFromPoint(linebounds.Max);

	if (hit.SearchCollision & 0x0000FFFF)
	{
		for (i16 x = mincoord.x; x <= maxcoord.x; x++)
		{
			for (i16 y = mincoord.y; y <= maxcoord.y; y++)
			{
				for (i16 z = mincoord.z; z <= maxcoord.z; z++)
				{
					u64 coord = KCellCoordinate(x, y, z).ToU64();
					if (KGridCell* cell = StaticGrid.GetCell(coord))
					{
						trace.TestNode(cell->StaticCollision.GetHead());
					}
				}
			}
		}
	}

#if !_COMPILER
	if (hit.SearchCollision >= ECollisionMask::PlayerCharacter) // player character is the first bbox channel
	{	
		KCellMap* cellMap = &EntityGrid;
		if (compFrames > 0)
		{
			if (compFrames > MAX_SNAPSHOTS) compFrames = MAX_SNAPSHOTS;
			u32 frame = KTime::FrameCount() - compFrames;
			cellMap = &(GetNetState()->PositionSnapshots[frame % MAX_SNAPSHOTS]);
		}

		mincoord = cellMap->CoordinateFromPoint(linebounds.Min);
		maxcoord = cellMap->CoordinateFromPoint(linebounds.Max);

		KCellCoordinate startCoord = cellMap->CoordinateFromPoint(line.a);

		for (i16 x = mincoord.x; x <= maxcoord.x; x++)
		{
			for (i16 y = mincoord.y; y <= maxcoord.y; y++)
			{
				for (i16 z = mincoord.z; z <= maxcoord.z; z++)
				{
					u64 coord = KCellCoordinate(x, y, z).ToU64();
					if (KGridCell* cell = cellMap->GetCell(coord))
					{
						KTimePoint entStart = KTime::Now(); 
						
						if (compFrames == 0)
						{
							for (KEntProp_CollidableBBox* box : cell->Entities)
							{
								KEntity* boxEnt = box->GetEntity();
								GVec3 pos = boxEnt->GetPosition();
								trace.TestEntity(box, pos);
							}
						}
						else
						{
							for (KCompEntity& ent : cell->CompEntities)
							{
								if (KEntity* e = ent.Entity.Get())
								{
									KEntProp_CollidableBBox* box = e->As<KEntProp_CollidableBBox>();
									EEntTraceResult result = trace.TestEntity(box, ent.Position);

									// see if entity moved into this object instead
									// see if we can possibly collide with this entity
									if (result < EEntTraceResult::TEST_PREV_FRAME) continue;

									// entity must have moved through our start point
									if (coord != startCoord) continue;

									// dont try this if the entity teleported to the more recent position
									if (ent.Flags & KCompEntity::Teleported)
										continue;

									// trace the last move of the potential target
									// its possible they walked into this during their move
									if (ent.Position != ent.PrevPosition)
									{
							
										GHitResult moveHit;
										box->InitHitResult(moveHit);
										//moveHit.SearchCollision = hit.TraceCollision;
										//moveHit.BoxEntity = box;
										//moveHit.TraceCollision = box->GetCollisionChannels();
										//moveHit.PositionOffsetZ = ent.PositionOffsetZ;
										
										/*const auto isThis = [](KTraceHitParams& hits, KCompTraceLocalCapture& locals) -> bool
										{
											return ((KEntProp_CollidableBBox*)hits.Test->Object) == locals.ThisBox;
										};

										KCompTraceLocalCapture cap;
										cap.ThisBox = hit.BoxEntity;
										KFunctionArg<KTraceHitParams, KCompTraceLocalCapture, bool> func(isThis, cap);
										KBvhGrid::TraceBox(GLineSegment(ent.PrevPosition, ent.Position), box->GetCollisionBoundsHalfExtent(), moveHit, &func);*/

										//GBoundingBox extendedBounds = hit.BoxEntity->GetCollisionBoundsHalfExtent();
										//extendedBounds += box->GetCollisionBoundsHalfExtent();

										GLineSegment otherLine(ent.PrevPosition, ent.Position);
										GVec3 otherExtent = box->GetCollisionBoundsHalfExtent();
										otherExtent.z += moveHit.PositionOffsetZ;

										KBoxTrace t;
										t.OutFraction = 1;
										t.Hit = &moveHit;
										t.Line = &otherLine;
										t.TraceBounds.Update(otherLine.a);
										t.TraceBounds.Update(otherLine.b);
										t.Extent = otherExtent;
										t.TraceBounds.Min -= otherExtent;
										t.TraceBounds.Max += otherExtent;
										t.SquaredDistance = otherLine.a.DistanceSq(otherLine.b);
										
										KEntProp_CollidableBBox* cbox = const_cast<KEntProp_CollidableBBox*>(hit.BoxEntity);
										GVec3 cpos = cbox->GetEntity()->GetPosition();
										EEntTraceResult r = t.TestEntity(cbox, cpos);

										if (moveHit.bHit)
										{
											// the entity moved into us during its move
											// since it happened before the move, assume this is the first contact that happened										
											hit.bHit = GHitResult::Block | GHitResult::BoxEnt;
											hit.Point = cpos;
											hit.Normal = -moveHit.Normal;		
											hit.Time = 0;
											hit.Object = box;
											hit.HitCollision = moveHit.TraceCollision;
											goto breakLoop;
										}
									}
								}
							}
						}

						TraceTimeInEnt += KTime::Since(entStart);
					}
				}
			}
		}
	}
breakLoop:
#endif

	TotalTraceTime += KTime::Since(traceStart);
	hit.Point.z -= hit.PositionOffsetZ;
	return hit.bHit;
}

template <typename Args, typename Locals, typename Return>
void KBvhGrid::PerformOnOverlapping(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<Args, Locals, Return>* func, u8 compFrames)
{
#if !_COMPILER
	GBoundingBox bounds(origin - halfExtent, origin + halfExtent);

	GHitResult hit;
	hit.SearchCollision = MAX_U32;

	// evil and slow heap allocation, need to find a way around this
	TSet<KCompEntity> ents;

	KCellMap* cellMap = &EntityGrid;

	if (compFrames > 0)
	{
		if (compFrames > MAX_SNAPSHOTS) compFrames = MAX_SNAPSHOTS;
		u32 frame = KTime::FrameCount() - compFrames;
		cellMap = &(GetNetState()->PositionSnapshots[frame % MAX_SNAPSHOTS]);
	}

	KCellCoordinate mincoord = cellMap->CoordinateFromPoint(origin - halfExtent);
	KCellCoordinate maxcoord = cellMap->CoordinateFromPoint(origin + halfExtent);
	
	for (i16 x = mincoord.x; x <= maxcoord.x; x++)
	{
		for (i16 y = mincoord.y; y <= maxcoord.y; y++)
		{
			for (i16 z = mincoord.z; z <= maxcoord.z; z++)
			{
				u64 coord = KCellCoordinate(x, y, z).ToU64();
				if (KGridCell* cell = cellMap->GetCell(coord))
				{
#if !_COMPILER 
					if (compFrames == 0)
					{
						for (KEntProp_CollidableBBox* box : cell->Entities)
						{
							if (bounds.Overlaps(box->GetAdjustedBounds()))
							{
								KEntity* ent = box->GetEntity();
								KCompEntity e;
								e.Entity = ent;
								e.Position = ent->GetPosition(); 
								ents.insert(e);
							}
						}
					}
					else
					{
						for (KCompEntity& e : cell->CompEntities)
						  if (KEntity* ent = e.Entity.Get())
						    if (KEntProp_CollidableBBox* box = ent->As<KEntProp_CollidableBBox>())
							  if (bounds.Overlaps(box->GetCollisionBounds() + e.Position))
								ents.insert(e);
					}
#endif
				}
			}
		}
	}

	for (const KCompEntity& e : ents)
	{
		if (KEntity* ent = e.Entity.Get())
		{
			TObjRef<KEntity> entRef = ent;

			ent->SetCompedPositionOffset(e.Position - ent->GetPosition());
			if (func->Execute(ent)) return;

			// might have killed it
			if (entRef.IsValid())
				ent->ResetCompedPositionOffset();
		}
	}
#endif // _COMPILER
}

void KBvhGrid::BuildFromPending()
{
	for (KPendingCollisionBrush& col : PendingCollision)
	{
		// add bounding planes
		col.Bounds.GetPlanes(col.Planes);
	
		// add face planes
		for (const auto& face : col.Faces)
		  if (!col.HasPlane(face.Plane))
			col.Planes.push_back(face.Plane);

		UPtr<KCollisionBrush> b = std::make_unique<KCollisionBrush>(col);
		CollisionBrushes.push_back(std::move(b));
	}

	BuildTrees();
}

void KBvhGrid::BuildTrees()
{
	// find valid grid cells from the collision brushes
	GFlt dim = StaticGrid.GetCellDimension();

	// track what cell coordinates contain what brushes
	THashMap<u64, TVector<KCollisionBrush*>> celldata;

#if _COMPILER
	KTerminalProgressBar progress1("Filling Grids");
	u32 count = 0;
#endif
	for (UPtr<KCollisionBrush>& col : CollisionBrushes)
	{
		TVector<KCellCoordinate> coords;
		StaticGrid.GetCoordsFromBounds(col->Bounds, coords);
		for (KCellCoordinate& coord : coords)
			celldata[coord.ToU64()].push_back(col.get());

#if _COMPILER
		count++;
		progress1.UpdateProgress(count, CollisionBrushes.size());
#endif
	}

#if _COMPILER
	progress1.Finish();
#endif

	// we now have all grid cells with the collisions that sit in them

#if _COMPILER
	KTerminalProgressBar progress2("Building Trees");
	count = 0;
#endif 
	for (auto& kv : celldata)
	{
		// build bvh from brush vector
		AddBvh(kv.first, kv.second);

#if _COMPILER
		count++;
		progress2.UpdateProgress(count, celldata.size());
#endif
}
#if _COMPILER
	progress2.Finish();
#endif
}

/*
bool KBvhGrid::TraceCylinder(const GLineSegment& line, flt_t halfHeight, flt_t radius, GHitResult& hit)
{
	hit.Reset();
	hit.Point = line.b;

	KCellCoordinate a = CoordinateFromPoint(line.a);
	KCellCoordinate b = CoordinateFromPoint(line.b);

	GBoundingBox linebounds;
	linebounds.Update(line.a);
	linebounds.Update(line.b);

	GVec3 extent(radius, radius, halfHeight);
	linebounds.Min -= extent;
	linebounds.Max += extent;

	//BvhMap[0].TraceBox(line, extent, hit);


	KCellCoordinate mincoord = CoordinateFromPoint(linebounds.Min);
	KCellCoordinate maxcoord = CoordinateFromPoint(linebounds.Max);

	for (i16 x = mincoord.x; x <= maxcoord.x; x++)
	  for (i16 y = mincoord.y; y <= maxcoord.y; y++)
		for (i16 z = mincoord.z; z <= maxcoord.z; z++)
		  BvhMap[KCellCoordinate(x, y, z).ToU64()].TraceCylinder(line, halfHeight, radius, hit);

	return hit.bHit;
}
*/

template bool KBvhGrid::TraceLine<KTraceHitParams, KMoveTraceLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KMoveTraceLocalCapture, bool>* condition, u8 flags);
template bool KBvhGrid::TraceBox<KTraceHitParams, KMoveTraceLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KMoveTraceLocalCapture, bool>* condition, u8 flags);

template bool KBvhGrid::TraceLine<KTraceHitParams, int, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, int, bool>* condition, u8 flags);
template bool KBvhGrid::TraceBox<KTraceHitParams, int, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, int, bool>* condition, u8 flags);

template bool KBvhGrid::TraceLine<KTraceHitParams, KCheckWaterLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KCheckWaterLocalCapture, bool>* condition, u8 flags);
template bool KBvhGrid::TraceBox<KTraceHitParams, KCheckWaterLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KCheckWaterLocalCapture, bool>* condition, u8 flags);

template bool KBvhGrid::TraceLine<KTraceHitParams, KPlaneTestLocalCapture, bool>(const GLineSegment& line, GHitResult& hit, KFunctionArg<KTraceHitParams, KPlaneTestLocalCapture, bool>* condition, u8 flags);
template bool KBvhGrid::TraceBox<KTraceHitParams, KPlaneTestLocalCapture, bool>(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KFunctionArg<KTraceHitParams, KPlaneTestLocalCapture, bool>* condition, u8 flags);

template bool KBvhGrid::TraceLine<KTraceHitParams, KCompTraceLocalCapture, bool>(const GLineSegment & line, GHitResult & hit, KFunctionArg<KTraceHitParams, KCompTraceLocalCapture, bool>*condition, u8 flags);
template bool KBvhGrid::TraceBox<KTraceHitParams, KCompTraceLocalCapture, bool>(const GLineSegment & line, const GVec3 & extent, GHitResult & hit, KFunctionArg<KTraceHitParams, KCompTraceLocalCapture, bool>*condition, u8 flags);

template void KBvhGrid::PerformOnOverlapping<KEntity*, KProjectileExplodeLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KProjectileExplodeLocalCapture, bool>* func, u8 compFrames);
template void KBvhGrid::PerformOnOverlapping<KEntity*, KTelefragLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KTelefragLocalCapture, bool>* func, u8 compFrames);
template void KBvhGrid::PerformOnOverlapping<KEntity*, KCheckPickupLocalCapture, bool>(const GVec3& origin, const GVec3& halfExtent, KFunctionArg<KEntity*, KCheckPickupLocalCapture, bool>* func, u8 compFrames);







#if 0

#include <cmath>
#include <iostream>
#include <vector>

struct Vector3 {
	float x, y, z;
};

struct GridCell {
	int x, y, z;
};

class Grid {
public:
	Grid(int size) : size_(size) {}

	void TraceLine(const Vector3& start, const Vector3& end,
		std::function<void(const GridCell&)> callback) {
		// Calculate the number of steps to take along the line
		int steps = ceil(fmax(fabs(end.x - start.x), fmax(fabs(end.y - start.y), fabs(end.z - start.z))) / size_);

		// Calculate the step vector
		Vector3 step_vec;
		step_vec.x = (end.x - start.x) / steps;
		step_vec.y = (end.y - start.y) / steps;
		step_vec.z = (end.z - start.z) / steps;

		// Initialize the current position to the start position
		Vector3 cur_pos = start;

		// Loop over the steps
		for (int i = 0; i <= steps; i++) {
			// Calculate the grid cell that the current position is in
			GridCell cell;
			cell.x = floor(cur_pos.x / size_);
			cell.y = floor(cur_pos.y / size_);
			cell.z = floor(cur_pos.z / size_);

			// Call the callback function with the current cell
			callback(cell);

			// Check if the line touches or crosses the x, y, and z boundaries of any other cells
			CheckCellBoundary(cur_pos, cell, size_, step_vec, callback);

			// Increment the current position by the step vector
			cur_pos.x += step_vec.x;
			cur_pos.y += step_vec.y;
			cur_pos.z += step_vec.z;
		}
	}

#endif