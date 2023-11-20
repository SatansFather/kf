#pragma once

#if !_SERVER

#include "entity.h"
#include "properties/movable.h"
#include "properties/move_states.h"
#include "properties/controllable.h"
#include "properties/collidable.h"

class KEntity_DeathCamera : public KEntity,
	public KEntProp_Movable,
	public KMoveState_Physics,
	public KEntProp_CollidableBBox
{
	
public:
	
	bool bHasHit = false;
	GFlt OffsetZ = 0;

	KEntity_DeathCamera();
	~KEntity_DeathCamera();
	void OnMoveBlocked(const GVec3& preVel, const GHitResult& hit) override;
	void Tick() override;
};

#endif