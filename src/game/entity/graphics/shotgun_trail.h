#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_ShotgunTrail : public KEntity, 
public KEntProp_Renderable<KSmokeBeam>
{
	
	f32 RandomTimeOffset = 0;

public:
	
	FColor8 Color;
	f32 Scale = 2;
	f32 Lifetime = 1;
	f32 TimeCreated;

	GVec3 StartPos, EndPos, LastEndPos;

	GVec3 LastPos;
	f32 LastScale;
	KEntity_ShotgunTrail();

	void Tick() override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KSmokeBeam& entry) override;
#endif

};

#endif