#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_BulletHole : public KEntity,
	public KEntProp_Renderable<KBulletHole>
{
	FVec3 Position, Normal;
	f32 RenderTimeCreated = 0;

public:

	static void Create(const GVec3& position, const GVec3& normal);

	KBufferUpdateResult UpdateBuffers(KBulletHole& entry) override;

};

#endif