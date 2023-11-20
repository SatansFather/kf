#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/audio/sound_list.h"

struct KPickupMessage
{
	KString Text;
	static void Create(const KString& text);
};

#endif