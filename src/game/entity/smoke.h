/*
#pragma once

#if !_SERVER
#include "entity.h"
#include "properties/renderable.h"

class KEntity_DynamicSmoke : public KEntity, 
	public KEntProp_Renderable<KDynamicSmokeParticle>
{
	
	f32 RandomTimeOffset = 0;

public:
	
	f32 R = 1, G = 1, B = 1, A = 1;
	f32 Scale = 16;
	f32 Lifetime = 1;

	GVec3 LastPos;
	f32 LastScale;
	KEntity_DynamicSmoke();

	void Tick() override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KDynamicSmokeParticle& entry) override;
#endif

};
#endif*/