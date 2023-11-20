#pragma once

#include "entity.h"
#include "properties/movable.h"
#include "properties/move_states.h"
#include "properties/controllable.h"
#include "properties/collidable.h"

class KEntity_Spectator : public KEntity,
	public KEntProp_Movable,
	public KMoveState_Flying,
	public KEntProp_CollidableBBox,
	public KEntProp_Controllable
{
	
	u8 LastInputState = 0;
	GVec3 EntVelocity;
	GVec3 LastFramePosition;

	bool bCyclingSpecPoints = false;
	u32 SpecPointIndex = 0;
	f32 SpecPointAlpha = 0;

public:

	u8 ViewedPlayerIndex = 255;
	
	KEntity_Spectator();
	void Tick() override;

	GVec3 GetEntVelocity() const { return EntVelocity; }
	KEntity* GetViewedEntity();
	bool ForceMaintainPortalRotation() const override { return false; }

	void CycleSpecPoints();
};