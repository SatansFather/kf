#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_LightFlash : public KEntity,
	public KEntProp_Renderable<KLightFlash>
{
public:

	KLightFlash FlashData;
	TObjRef<KEntity> PositionEntity;

	static TObjRef<KEntity_LightFlash> CreateFlash(const GVec3& position, FColor8 color, f32 radius, f32 growTime, f32 dimTime, f32 falloff = 1);

	KBufferUpdateResult UpdateBuffers(KLightFlash& entry) override;

};

#endif