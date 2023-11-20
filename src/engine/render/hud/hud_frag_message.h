#pragma once

#if !_SERVER && !_COMPILER

#include "hud_widget.h"
#include "../../game/frag_message.h"

class KHudFragMessage : public KHudWidget
{
	friend class KRenderInterface;

	u32 YouFraggedLayout = 0;
	u32 FraggedByLayout = 0;
	u32 KillerLayout = 0;
	u32 VictimLayout = 0;

	KString KillerName;
	KString VictimName;

	KFragMessage CurrentFragMessage;
	KDeathMessage CurrentDeathMessage;

public:

	KHudFragMessage();
	void Draw() override;
};

#endif