#pragma once

#include "kfglobal.h"
#include "engine/audio/sound_list.h"

struct KOnScreenMessage
{
	KString Text;
	u32 Slot = 0;
	f32 Duration = 4;
	u32 TextLayout = 0;

	static void Create(const KString& text, u32 slot = 0, KSoundID sound = KSoundID::Null_Sound, f32 duration = 4);
};