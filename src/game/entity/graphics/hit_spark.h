#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_HitSpark : public KEntity, 
public KEntProp_Renderable<KHitSpark>
{

public:
	

	FVec3 Position, Normal, ReflectedVelocity;
	f32 RenderTimeCreated = 0;
	f32 RandomOffset = 0;

	KEntity_HitSpark();
	static void Create(const GVec3& pos, const GVec3& norm);

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KHitSpark& entry) override;
#endif

};

#endif