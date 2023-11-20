#pragma once

#include "entity.h"
#include "properties/renderable.h"

class KEntity_BlastExplosion : public KEntity

#if !_SERVER
	, public KEntProp_Renderable<KBlastExplosion>
#endif
{
	f32 RenderTimeCreated = 0;

public:

	KEntity_BlastExplosion();

	static void CreateExplosion(const GVec3& pos);

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KBlastExplosion& entry) override;
#endif
};