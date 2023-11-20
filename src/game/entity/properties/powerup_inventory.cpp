#include "powerup_inventory.h"
#include "../powerup_index.h"
#include "../pickups/powerups/pickup_powerup_brain.h"
#include "engine/game_instance.h"
#include "../pickups/powerups/pickup_powerup_invis.h"
#include "engine/game/match.h"
#include "../deferred_drop.h"

KEntProp_PowerupInventory::~KEntProp_PowerupInventory() 
{
	if (IsNetAuthority())
	{
		if (KGameInstance::Get().IsDestroyingMatch())
			return;

		for (u8 i = 0; i < EPowerupID::NumPowerups; i++)
		{
			TObjRef<KEntity_Powerup>& p = CarriedPowerups[i];
			if (KEntity_Powerup* p = CarriedPowerups[i].Get())
			{
				u32 frames = p->GetRemainingFrames();
				if (frames > KTime::FramesFromTime(1))
				{
					KDeferredDropSpawn d;
					d.Position = p->GetPosition();
					d.ItemType = KDeferredDropSpawn::Type_Powerup;
					d.FrameDuration = frames;
					d.PowerupIndex = i;
					d.Finalize();
				}
			}
		}
	}
}

void KEntProp_PowerupInventory::GetPowerupFrameCountArray(u16* arr)
{
	for (u8 i = 0; i < EPowerupID::NumPowerups; i++)
	{
		if (CarriedPowerups[i].IsValid())
			arr[i] = CarriedPowerups[i].Get()->GetRemainingFrames();
		else 
			arr[i] = 0;
	}
}

u16 KEntProp_PowerupInventory::GetPowerupFramesRemaining(u8 id) const
{
	if (CarriedPowerups[id].IsValid())
		return CarriedPowerups[id].Get()->GetRemainingFrames();

	return 0;
}

bool KEntProp_PowerupInventory::HasPowerup(u8 id) const
{
	return GetPowerupFramesRemaining(id) > 0;
}
