#pragma once

#include "engine/math/vec3.h"

/*
// move modes are related with a set of movement properties
// walking mode is shared with falling even though their state is separate
enum class EMoveMode : u8
{
	Walking, Flying, Swimming
};
*/

// move state is a real time capture
// falling and walking are part of the same mode but a different state
enum class EMoveState : u8
{
	Walking, 
	Flying, 
	Swimming, 
	Falling,
	Physics
};

struct KMovementSurfaceModifier
{
	f32 SpeedMod = 1;
	f32 AccelMod = 1;
	f32 DecelMod = 1;
};

struct KMovementProperties
{
	f32 MoveSpeed = 0;
	f32 Acceleration = 0;
	f32 Deceleration = 0;

	void ScaleWithSurface(const KMovementSurfaceModifier& surf)
	{
		MoveSpeed *= surf.SpeedMod;
		Acceleration *= surf.AccelMod;
		Deceleration *= surf.DecelMod;
	}
};