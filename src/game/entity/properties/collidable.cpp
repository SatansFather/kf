#include "collidable.h"
#include "engine/game/match.h"
#include "engine/collision/broadphase/bvh_grid.h"
#include "game/entity/entity.h"
#include "movable.h"
#include "engine/collision/trace.h"
#include "killable.h"
#include "../../../engine/net/player.h"
#include "../character.h"
#include "../graphics/gibs.h"

KEntProp_CollidableBBox::~KEntProp_CollidableBBox()
{
	DisableCollision();
}

void KEntProp_CollidableBBox::EnableCollision()
{
	bool update = false;
	if (!bCollisionEnabled) update = true;
	bCollisionEnabled = true;
	if (update) UpdateOccupiedCells();
}

void KEntProp_CollidableBBox::DisableCollision()
{
	bool update = false;
	if (bCollisionEnabled) update = true;
	bCollisionEnabled = false;
	if (update) UpdateOccupiedCells();
}

void KEntProp_CollidableBBox::SetCollisionBoundsHalfExtent(const GVec3& e, bool skipUpdate /*= false*/)
{
	CollisionBounds.Min = -e;
	CollisionBounds.Max = e;
	CollisionBounds.bSet = true;

	if (!skipUpdate)
		UpdateOccupiedCells();
}

const GVec3& KEntProp_CollidableBBox::GetCollisionBoundsHalfExtent() const
{
	return CollisionBounds.Max;
}

const GBoundingBox& KEntProp_CollidableBBox::GetCollisionBounds() const
{
	return CollisionBounds;
}

GBoundingBox KEntProp_CollidableBBox::GetAdjustedBounds()
{
	GBoundingBox b = GetCollisionBounds();
	b.Min += CrouchDistance / 2;
	b.Max -= CrouchDistance / 2;
	return b + GetEntity()->GetPosition().AdjustZ(-CrouchDistance / 2);
}

GFlt KEntProp_CollidableBBox::GetLargestDimension() const
{
	return KMax(KMax(CollisionBounds.Max.x, CollisionBounds.Max.y), CollisionBounds.Max.z);
}

void KEntProp_CollidableBBox::UpdateOccupiedCells()
{
	if (!GetGameMatch()) return;
	if (CollisionChannels == 0) return;

	if (KEntProp_Movable* m = GetEntity()->As<KEntProp_Movable>())
	{
		// comped movers test if other ents moved into them on previous frames
		if (m->CompFrames > 0)
			return;
	}

	KBvhGrid* grid = GetGameMatch()->Grid.get();

	for (u32 i = 0; i < GridOverlapCount; i++)
		grid->EntityGrid.GetCell(GridOverlaps[i])->RemoveEntity(this);
		
	if (!IsCollisionEnabled())
		return;

	grid->EntityGrid.GetCoordsFromBounds(GetAdjustedBounds(), GridOverlaps, GridOverlapCount);

	for (u32 i = 0; i < GridOverlapCount; i++)
		grid->EntityGrid.AddCell(GridOverlaps[i])->AddEntity(this);
}

void KEntProp_CollidableBBox::InitHitResult(GHitResult& hit, bool addBlocks /*= true*/, bool addOverlaps /*= true*/) const
{
	hit.TraceCollision = CollisionChannels;
	hit.SearchCollision = 0;
	if (addBlocks) hit.SearchCollision |= CollisionBlocks;
	if (addOverlaps) hit.SearchCollision |= CollisionOverlaps;
	hit.PositionOffsetZ = -CrouchDistance/2;
	hit.BoxEntity = this;
}

void KEntProp_CollidableBBox::AssignIgnoreID(u32 id /*= 0*/)
{
	if (KGameMatch* match = GetGameMatch())
	{
		if (id == 0) 
		{
			match->GlobalCollisionIgnoreID++;
			IgnoreID = match->GlobalCollisionIgnoreID;
		}
		else
			IgnoreID = id;
	}
}

bool KEntProp_CollidableBBox::MatchingIgnoreID(KEntProp_CollidableBBox* other)
{
	return MatchingIgnoreID(other->IgnoreID);
}

bool KEntProp_CollidableBBox::MatchingIgnoreID(u32 id)
{
	return IgnoreID	> 0 && IgnoreID == id;
}

void KEntProp_CollidableBBox::AttemptTelefrag()
{
	if (!IsNetAuthority()) return;
	if (!CanTelefrag()) return;

	// TODO potential crash here

	const auto telefrag = [](KEntity*& ent, KTelefragLocalCapture& locals) -> bool
	{
		if (!ent->CanBeTelefragged()) return false;
		if (ent == locals.ThisEnt) return false;

		if (KEntProp_Killable* k = ent->As<KEntProp_Killable>())
		{
			KNetPlayer* player = locals.ThisEnt->GetOwningPlayer();
			k->SetHealth(-50, true, player);
		}

		return false;
	};

	KTelefragLocalCapture cap;
	cap.ThisBox = this;
	cap.ThisEnt = GetEntity();

	KFunctionArg<KEntity*, KTelefragLocalCapture, bool> func(telefrag, cap);

	PerformOnOverlapping(cap.ThisEnt->GetPosition(), GetCollisionBoundsHalfExtent(), &func, 0);
}
