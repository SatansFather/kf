#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_WaterSplash : public KEntity, 
public KEntProp_Renderable<KWaterSplash>
{

public:
	

	FVec3 Position, Normal, ReflectedVelocity;
	f32 RenderTimeCreated = 0;
	f32 RandomOffset = 0;
	f32 Scale = 1, Strength = 1;
	FColor8 Color;

	KEntity_WaterSplash();
	static void Create(const GVec3& pos, const GVec3& norm, FColor8 color, f32 scale, f32 strength);

	KBufferUpdateResult UpdateBuffers(KWaterSplash& entry) override;
};

#endif