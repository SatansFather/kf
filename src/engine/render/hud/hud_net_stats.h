#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"
#include "../../net/net_stats.h"

class KHudNetStats : public KHudWidget
{
	friend class KRenderInterface;

	f32 Top, Left, Bottom, Right;

	KNetStats NetStats;
	KNetStats LastNetStats;

	u32 FrameDiffHandle = 0;
	u32 FrameDiffValueHandle = 0;
	u32 TimeDilationHandle = 0;
	u32 TimeDilationValueHandle = 0;
	u32 ActualDelayHandle = 0;
	u32 ActualDelayValueHandle = 0;
	u32 InBytesHandle = 0;
	u32 InBytesValueHandle = 0;
	u32 OutBytesHandle = 0;
	u32 OutBytesValueHandle = 0;

	void DrawBackground();
	void Draw() override;

	void OnWindowResize() override;

public:
	KHudNetStats();
	~KHudNetStats();
};

#endif