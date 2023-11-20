#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"

#define TICK_FRAMES_TRACKED 10

class KHudGameFrameStats : public KHudWidget
{
	friend class KRenderInterface;

	f64 LastTickTime[TICK_FRAMES_TRACKED];
	f64 LastNetTime[TICK_FRAMES_TRACKED];
	f64 LastCopyTime[TICK_FRAMES_TRACKED];
	f64 LastRenderUpdateTime[TICK_FRAMES_TRACKED];

	KTimePoint LastTextUpdateTime;
	u32 TextHandle = 0;

	void Draw() override;
	void OnWindowResize() override;

	void AddTickTime(f64 time);
	void AddNetTime(f64 time);
	void AddCopyTime(f64 time);
	void AddRenderUpdateTime(f64 time);

public:
	KHudGameFrameStats();
	~KHudGameFrameStats();
};

#endif