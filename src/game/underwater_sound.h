#pragma once

#if !_NOSOUND

#include "kfglobal.h"
#include "engine/audio/audio.h"

class KUnderwaterSound
{
	static KSoundInstance DeepInstance;
	static KSoundInstance ShallowInstance;

	static bool bUnderwaterLastFrame;

public:

	~KUnderwaterSound();

	static void Init();
	static void Update();
};
#endif