#pragma once

#include "kfglobal.h"
#include "../math/vec3.h"

struct KDamageNumber
{
	KDamageNumber() = default;
	KDamageNumber(const FVec3& pos, i32 damage, u32 frameLifetime);

	FVec3 Position;
	i32 Damage = 0; // negative is heal
	u32 FramesRemaining = 0;
	f64 RenderTimeCreated = 0;
	u8 RandomSeed;
	f64 Lifespan;
};

#pragma pack(push, 1)
struct KReppedDamageNumber
{
	u32 Frame = 0;
	u32 NetID = 0;
	i32 Damage = 0;
};
#pragma pack(pop)
