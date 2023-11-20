#pragma once

#include "../powerup.h"
#include "engine/net/snapshottable.h"

#if !_SERVER
#include "../properties/renderable.h"
#endif

class KEntity_Powerup_Invis :
	public KEntity_Powerup
#if !_SERVER
	, public KEntProp_Renderable<KDynamicLight>
#endif
{
public:
	u32 LastBreakFrame = MAX_U32;
	bool bLastInvis = false;
	SNAP_PROP(bool, bIsInvisible = false)
	void OnRep_bIsInvisible();

	KEntity_Powerup_Invis();


#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KDynamicLight& entry) override;
#endif

public:	
	void SetInvis(bool invis);
	void Tick() override;
	bool IsInvisible() const { return bIsInvisible; }
	void Break();
};