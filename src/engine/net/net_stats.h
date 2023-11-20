#pragma once

#include "kfglobal.h"

struct KNetStats
{
	f32 TimeDilation = 1;
	f32 ClientFrameDiff = 0;
	f32 FrameTargetOffset = 1;
	f32 Ping = 0;
	f32 ActualDelay = 0;
	u32 OutBytes = 0;
	u32 InBytes = 0;
};