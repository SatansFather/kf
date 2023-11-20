#pragma once

#include "kfglobal.h"

struct KSavedMove
{
	u8 Input;
	GVec3 Position;
	GVec3 Velocity;
	GVec3 FloorNormal;
	u16 Pitch;
	u16 Yaw;
};