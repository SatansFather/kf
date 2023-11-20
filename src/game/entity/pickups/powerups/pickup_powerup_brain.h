#pragma once

#include "../pickup_powerup.h"
#include "../../properties/renderable.h"
#include "engine/audio/sound_instance.h"

class KEntity_Pickup_Powerup_Brain :
	public KEntity_Pickup_Powerup
#if !_SERVER
	, public KEntProp_Renderable<KPowerupBrain, KDynamicLight>
#endif
{
public:

	KSoundInstance AmbientSound;

	KEntity_Pickup_Powerup_Brain();

	TObjRef<KEntity_Powerup> CreatePowerup() override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KPowerupBrain& entry, KDynamicLight& light) override;
#endif

	KSoundID GetDespawnSound() const override { return KSoundID::Despawn_Brain; }
	KSoundID GetRespawnSound() const override { return KSoundID::Respawn_Brain; }

	void Tick() override;

	KString GetPickupMessage() override { return "FIRING FRENZY"; }
};