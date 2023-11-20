#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_RocketTrail : public KEntity, 
public KEntProp_Renderable<KRocketTrail>
{

public:

	static TObjRef<KEntity_RocketTrail> Create(const GVec3& startPos, const GVec3& vel, bool offset, bool mine);

	FVec3 StartPosition;
	FVec3 Velocity;
	f32 TimeCreated = 0;
	f32 RocketDeathTime = 0;
	f32 LastRocketDeathTime = 0;
	bool bUseRenderOffset = false;
	bool bIsMyRocket = false;
	
	KEntity_RocketTrail();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KRocketTrail& entry) override;
#endif

};

#endif