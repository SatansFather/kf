#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"

class KHudFrameStats : public KHudWidget
{
	friend class KRenderInterface;

	u32 FrameRateHandle = 0;
	u32 FrameRateValueHandle = 0;
	u32 DrawCallHandle = 0;
	u32 DrawCallValueHandle = 0;
	u32 TriangleHandle = 0;
	u32 TriangleValueHandle = 0;

	u32 LastFrameRate = 0;
	u32 LastDrawCalls = 0;
	u32 LastTriangles = 0;

	void Draw() override;
	void OnWindowResize() override;

	// updates handle if changed
	void UpdateStatHandle(u32 current, u32& last, u32& handle);

	void DrawStat();

public:
	KHudFrameStats();
	~KHudFrameStats();
};

#endif