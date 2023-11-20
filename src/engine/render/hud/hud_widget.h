#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "../2d_prim.h"
#include "alignment.h"

// base widget class
class KHudWidget
{
	EVerticalAlignment VerticalAlignment;
	EHorizontalAlignment HorizontalAlignment;
	KHudRectF Bounds;

public:
	
	KHudWidget();

	void DrawTextShadowed(u32 handle, KHudPointF origin, f32 offset, struct FColor8 mainColor, struct FColor8 shadowColor, f32 scale = 1);

	virtual void OnWindowResize() {}
	virtual void Draw() = 0;
};

#endif