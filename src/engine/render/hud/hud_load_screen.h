#pragma once

#if !_SERVER

#include "hud_widget.h"

class KHudLoadScreen : public KHudWidget
{
	f32 LastProgress = 0;
	KString LoadingMapName;
	u32 LoadScreenHandle = 0;

public:

	void Draw() override;
	void OnWindowResize() override;
};

#endif