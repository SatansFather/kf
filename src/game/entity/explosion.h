#pragma once

#include "entity.h"
#include "properties/renderable.h"

class KEntity_Explosion : public KEntity

#if !_SERVER
	, public KEntProp_Renderable<KExplosion>
#endif
{
	f32 Radius = 150;
	FVec3 Normal;
	f32 RenderTimeCreated = 0;

public:

	KEntity_Explosion();

	static void CreateExplosion(f32 radius, const GVec3& pos, const GVec3& norm);

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KExplosion& entry) override;
#endif
};