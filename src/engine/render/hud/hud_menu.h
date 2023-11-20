#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"

class KHudMenu : public KHudWidget
{
	friend class KRenderInterface;

	TMap<class KBasicMenu*, TVector<u32>> OptionLayouts;
	TMap<class KBasicMenu*, u32> HeaderLayouts;

	u32 EnabledLayout, DisabledLayout, SliderLayout, SliderBarLayout;

public:
	KHudMenu();
private:
	void Draw() override;
	void AddMenuOptions(class KBasicMenu* menu);
};

#endif