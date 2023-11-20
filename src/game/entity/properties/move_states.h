#pragma once

#include "engine/math/vec3.h"
#include "../mover_info.h"

// move stats contain extra data for different movement modes
// inherit from these to enable a specific mode

class KMoveState_Walking
{
public:
	KMovementProperties WalkingProperties;
	GVec3 WalkingFloorNormal = GVec3(0, 0, 1);
	GVec3 LastFrameSteepestWalkingFloorNormal = GVec3(0, 0, 0);
	GVec3 CurrentFrameSteepestWalkingFloorNormal = GVec3(0, 0, 0);
	GFlt WalkableFloorAngle = 46;
	GFlt JumpHeight = 40;
	GFlt AirAcceleration;
	GFlt CrouchDropDistance = 16;
	u8 CrouchFrameCount = 12;
	~KMoveState_Walking() {}

	void UpdateFloorNormal(const GVec3& norm)
	{
		WalkingFloorNormal = norm;
		if (norm.z < CurrentFrameSteepestWalkingFloorNormal.z)
			CurrentFrameSteepestWalkingFloorNormal = norm;
	}

	GVec3 GetSteepestNormal()
	{
		const GVec3& last = LastFrameSteepestWalkingFloorNormal;
		const GVec3& current = CurrentFrameSteepestWalkingFloorNormal;
		
		if (current.z == 0) return last;
		if (last.z == 0) return current;

		return current.z < last.z ? current : last;
	}
};

class KMoveState_Swimming
{
public:
	KMovementProperties SwimmingProperties;
	~KMoveState_Swimming() {}
};

class KMoveState_Flying
{
public:
	KMovementProperties FlyingProperties;
	~KMoveState_Flying() {}
};

class KMoveState_Physics
{
public:
	KMovementProperties PhysicsProperties;
	KMovementProperties PhysicsFluidProperties;
	GFlt SinkSpeed = 350;
	GFlt Bounciness = 1;
	GVec3 PhysicsFloorNormal;
	~KMoveState_Physics() {}
};