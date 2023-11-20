#pragma once

#include "../ent_prop.h"
#include "kfglobal.h"
#include "engine/math/aabb.h"
#include "game/collision_mask.h"
#include "engine/collision/hit_result.h"

class KEntProp_CollidableBBox : public KEntProp
{
protected:

	u32 CollisionChannels = 0;
	u32 CollisionOverlaps = 0;
	u32 CollisionBlocks = 0;

private:

	bool bCollisionEnabled = true;
	GBoundingBox CollisionBounds;

	u64 GridOverlaps[8];
	u8 GridOverlapCount = 0;

public:

	u32 IgnoreID = 0;

	// adjustments for crouching
	GFlt CrouchDistance = 0;

public:

	~KEntProp_CollidableBBox();

	void EnableCollision();
	void DisableCollision();

	// uncrouched
	const GVec3& GetDefaultHalfExtent() const { return CollisionBounds.Max; }

	void SetCollisionBoundsHalfExtent(const GVec3& e, bool skipUpdate = false);

	const GVec3& GetCollisionBoundsHalfExtent() const;

	virtual bool IsCollisionEnabled() { return bCollisionEnabled; }
	const GBoundingBox& GetCollisionBounds() const;
	GBoundingBox GetAdjustedBounds();

	GFlt GetLargestDimension() const;

	u32 GetCollisionChannels() const { return CollisionChannels; }
	u32 GetCollisionOverlaps() const { return CollisionOverlaps; }
	u32 GetCollisionBlocks() const { return CollisionBlocks; }

	bool IsChannel(u32 channel) const { return CollisionChannels & channel; }
	bool OverlapsChannel(u32 channel) const { return CollisionOverlaps & channel; } 
	bool BlocksChannel(u32 channel) const { return CollisionBlocks & channel; } 

	void UpdateOccupiedCells();

	void InitHitResult(GHitResult& hit, bool addBlocks = true, bool addOverlaps = true) const;

	void AssignIgnoreID(u32 id = 0); // leave ID at 0 to increment
	bool MatchingIgnoreID(KEntProp_CollidableBBox* other);
	bool MatchingIgnoreID(u32 id);

	void AttemptTelefrag();

	virtual bool CanTelefrag() { return false; }

	u8 GetGridOverlapCount() const { return GridOverlapCount; }
	const u64* GetGridOverlaps() const { return GridOverlaps; }
};