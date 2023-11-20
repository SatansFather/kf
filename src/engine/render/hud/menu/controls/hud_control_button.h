#pragma once

#if !_SERVER && !_COMPILER

#include "hud_control_clickable.h"
#include "../../../font_usage.h"

class KHudControlButton : public KHudControl_Clickable
{
	friend class KRenderInterface;

	u32 TextLayoutHandle = 0;

public:

	KString ButtonText;
	EFontUsage FontUsage;

	void Draw() override;
	void SetText(const KString& text);
	void SetClickBoundsToText();

};

#endif