#pragma once

#if !_SERVER

#include "engine/math/vec3.h"

struct KCameraPush
{
	enum 
	{
		HasRotation		= 1,
		HasFov			= 2,
		HasShake		= 4,
		HasConcussion	= 8
	};
	u8 Flags;

	struct
	{
		KTimePoint Time;
		f32 Duration = 0;
		f32 Alpha = 0;
		FVec3 Rotation;
	} Rotation;

	struct 
	{
		KTimePoint Time;
		f32 Duration = 0;
		f32 Alpha = 0;
		f32 Fov = 0;
	} Fov;

	struct
	{
		KTimePoint Time;
		f32 Strength = 0;
		f32 Duration = 0;
		f32 Rate = 0;
	} Shake;

	f32 Concussion = 0;

	void PushRotation(const FVec3& amount, f32 duration, f32 startAlpha = 0);
	void PushFov(f32 amount, f32 duration, f32 startAlpha = 0);
	void PushShake(f32 strength, f32 duration, f32 shakeRate);
	void AddConcussion(f32 value);
	void UpdateForRender(KCameraPush& renderPush);
	void Reset();

	KCameraPush operator*(const f32 scale);
};

#endif