#pragma once

#include "../pickup.h"
#include "../powerup.h"

class KEntity_Pickup_Powerup : public KEntity_PickupBase
{

	friend class KEntProp_PowerupInventory;

protected:

	u8 PowerupIndex = 0;

public:

	SNAP_PROP_TRANSIENT(u32, DropDeathFrame = 0, SNAP_FIRST_ONLY)
	void GetTransient_DropDeathFrame(u32& val);
	void SetTransient_DropDeathFrame(u32& val);

	u32 FrameDuration = 0;

	virtual TObjRef<KEntity_Powerup> CreatePowerup() = 0;

	bool CanPickUp(KEntity* ent) override;
	void PickUp(KEntity* ent) override;
	void Tick() override;
};