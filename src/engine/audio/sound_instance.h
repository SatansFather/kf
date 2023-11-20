#pragma once

#include "kfglobal.h"

class KSoundInstance
{
	friend class KAudio;

	u32 Handle = 0;

public:

	u32 GetHandle() const { return Handle; }
};
