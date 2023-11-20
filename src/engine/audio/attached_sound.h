#pragma once

#include "kfglobal.h"
#include "sound_instance.h"
#include "game/entity/entity.h"

struct KAttachedSound
{
	KSoundInstance Instance;
	TObjRef<KEntity> AttachedEntity;
	bool bPlayAfterDeath = false;
};
