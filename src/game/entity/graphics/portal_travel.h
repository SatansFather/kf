#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_PortalTravel : public KEntity, 
public KEntProp_Renderable<KPortalTravel>
{

	FVec3 Position, HalfExtent;
	f32 RenderTimeCreated = 0;
	bool bIsEntry = false;
	
public:
	
	KEntity_PortalTravel();
	static void Create(const GVec3& pos, const GVec3& halfExtent, bool isEntry);

	KBufferUpdateResult UpdateBuffers(KPortalTravel& entry) override;
};

#endif