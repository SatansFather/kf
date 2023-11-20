#pragma once

#include "../pickup_powerup.h"
#include "../../properties/renderable.h"
#include "engine/net/snapshottable.h"


class KEntity_Pickup_Powerup_Invis :
	public KEntity_Pickup_Powerup
#if !_SERVER
	, public KEntProp_Renderable<KPowerupBrain, KDynamicLight>
#endif
{
public:

	KEntity_Pickup_Powerup_Invis();

	TObjRef<KEntity_Powerup> CreatePowerup() override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KPowerupBrain& entry, KDynamicLight& light) override;
#endif

	KString GetPickupMessage() override { return "INVISIBILITY"; }
};