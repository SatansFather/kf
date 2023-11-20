#pragma once

#if !_SERVER

#include "kfglobal.h"

struct KFrameStats
{
	// per period
	u32 FrameRate = 0;
	f32 FrameTime = 0;

	// per frame
	u32 DrawCalls = 0;
	u32 Triangles = 0;

	KTimePoint LastReset = KTime::Init();
	u32 FramesSinceReset = 0;

	void ResetFrame();
	void ResetPeriod();
	void UpdatePeriod();
};

#endif