#pragma once

#include "../ent_prop.h"
#include "kfglobal.h"
#include "../powerup_index.h"
#include "../powerup.h"
#include "../pickups/pickup_powerup.h"

class KEntProp_PowerupInventory : public KEntProp
{
	friend class KEntity_Pickup_Powerup;

public:

	TObjRef<KEntity_Powerup> CarriedPowerups[EPowerupID::NumPowerups];

	void GetPowerupFrameCountArray(u16* arr);
	u16 GetPowerupFramesRemaining(u8 id) const;
	bool HasPowerup(u8 id) const;

	~KEntProp_PowerupInventory();
};