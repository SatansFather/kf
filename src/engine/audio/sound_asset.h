#pragma once

#if !_NOSOUND

#include "kfglobal.h"
#include "soloud_wav.h"
#include "../global/paths.h"

class KSoundAsset
{
public:

	SoLoud::Wav Wave;	
	void LoadWave(const KString& file);
};

#else

class KSoundAsset {};

#endif
