#pragma once

#if !_SERVER && !_COMPILER

#include "hud_text_console.h"

class KHudChatConsole : public KHudTextConsole
{
	friend class KRenderInterface;
public:
	KHudChatConsole();
private:
	void GetBackgroundBounds(KHudRectF& rect) override;
	f32 GetMessageOpacity(class KTextMessage* msg) override;
};

#endif