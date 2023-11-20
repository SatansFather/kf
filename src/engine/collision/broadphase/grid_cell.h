#pragma once

#include "kfglobal.h"
#include "static_bvh.h"
#include "game/entity/entity.h"

struct KCompEntity
{
	TObjRef<KEntity> Entity;
	GVec3 Position;
	GVec3 PrevPosition;
	GFlt PositionOffsetZ = 0;

	enum 
	{
		Teleported = 1,
	};

	u8 Flags = 0;

	bool operator<(const KCompEntity& other) const
	{
		return Entity < other.Entity;
	}
};

// a cell in a spacial grid
class KGridCell
{
	friend class KBvhGrid;

	KStaticBVH StaticCollision;
	THashSet<class KEntProp_CollidableBBox*> Entities;

	TVector<KCompEntity> CompEntities;

public:

	KGridCell();

	void AddEntity(class KEntProp_CollidableBBox* ent);
	void RemoveEntity(class KEntProp_CollidableBBox* ent);

	void AddCompEntity(KCompEntity& ent);
};