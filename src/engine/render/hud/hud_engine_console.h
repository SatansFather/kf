#pragma once

#if !_SERVER && !_COMPILER

#include "hud_text_console.h"

class KHudEngineConsole : public KHudTextConsole
{
	friend class KRenderInterface;
public:
	KHudEngineConsole();
	void Draw() override;
private:
	void GetBackgroundBounds(KHudRectF& rect) override;
	f32 GetMessageOpacity(class KTextMessage* msg) override;

protected:

	void GetAutoSuggestions(TVector<KString>& vec, KTimePoint& time) override;
};

#endif