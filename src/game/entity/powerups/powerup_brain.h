#pragma once

#include "../powerup.h"
#include "engine/net/snapshottable.h"

#if !_SERVER
#include "../properties/renderable.h"
#endif
#include "engine/audio/sound_instance.h"

class KEntity_Powerup_Brain :
	public KEntity_Powerup
#if !_SERVER
	, public KEntProp_Renderable<KDynamicLight>
#endif
{
public:
	KSoundInstance CarrySound;

	KEntity_Powerup_Brain();

	void Tick() override;
	void OnEntityDestroyed() override;
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KDynamicLight& entry) override;
#endif

};