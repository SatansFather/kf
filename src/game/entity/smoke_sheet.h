#pragma once

#include "entity.h"
#include "properties/renderable.h"

class KEntity_SmokeSheet : public KEntity
#if !_SERVER
	, public KEntProp_Renderable<KSmokeSheet>
#endif
{
public:

	static void Create(const GVec3& pos);

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KSmokeSheet& entry) override;
#endif
};