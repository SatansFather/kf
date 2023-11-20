#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"


class KHudTargetName : public KHudWidget
{
	friend class KRenderInterface;

	u32 TextHandle;
	KString TargetPlayerName;
	KString LastTargetPlayerName;

public:

	KHudTargetName();
	~KHudTargetName();

private:

	void Draw() override;
};

#endif