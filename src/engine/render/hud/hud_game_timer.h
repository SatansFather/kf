#pragma once

#if !_SERVER

#include "hud_widget.h"
#include "game/entity/powerup_index.h"

class KHudGameTimer : public KHudWidget
{
	u32 SecondsLayout = 0;
	u32 MinutesLayout = 0;
	u32 PrevSeconds = MAX_U32;
	u32 PrevMinutes = MAX_U32;
	f32 SecondsWidth, MinutesWidth;

public:

	KHudGameTimer();
	void Draw() override;
	void OnWindowResize() override;


};

#endif