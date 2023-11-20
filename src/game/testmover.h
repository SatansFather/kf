/*
#pragma once

#include "engine/system/mempool.h"
#include "../engine/math/vec3.h"
#include "../engine/utility/delegate_listen.h"

class KTestMover : public KPoolable
{
public:
	DVec3 Position = DVec3(0, 0, 100);
	DVec3 Velocity;
	f64 Speed = 550;
	u8 KeyState = 0;
	bool bFlying = true;
	i32 RemainingPushFrames = 0;
	bool bOnGround;
	DVec3 FloorNormal;
	DVec3 FloorPoint;
	f64 OffGroundZ = 0;
	DVec3 DesiredDirection;
	KTestMover();

	void Tick(flt_t delta) override;

	void UpdateKeyState(u8 state);

	DVec3 HitPoint, HitNormal;
};*/