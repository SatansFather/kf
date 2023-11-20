#pragma once

#if !_SERVER && !_COMPILER

#include "hud_widget.h"

class KHudPickupMessage : public KHudWidget
{
	friend class KRenderInterface;

	u32 Handle = 0;
	KTimePoint LastUpdateTime = KTime::Init();
	u32 Combo = 0;
	KString Text;
	KString LastDrawText;

public:

	KHudPickupMessage();
	void Draw() override;
};

#endif