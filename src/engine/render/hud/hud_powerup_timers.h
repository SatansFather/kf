#pragma once

#if !_SERVER

#include "hud_widget.h"
#include "game/entity/powerup_index.h"

class KHudPowerupTimers : public KHudWidget
{

	UPtr<class KHudBitmap> PowerupIcons[EPowerupID::NumPowerups];
	UPtr<class KHudBitmap> PowerupNoise;
	UPtr<class KHudBitmap> PowerupGlow;

	f64 LastPowerupUpdateTimes[EPowerupID::NumPowerups];

public:
	
	u16 FramesRemaining[EPowerupID::NumPowerups];

	KHudPowerupTimers();
	void Draw() override;

	void OnWindowResize() override;

private:

	void Reset();

};

#endif