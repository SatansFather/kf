#pragma once

#if !_SERVER && !_COMPILER

#include "hud_widget.h"
#include "../font_usage.h"
#include "../color.h"

class KHudTextConsole : public KHudWidget
{
	friend class KRenderInterface;

protected:

	class KTextConsole* Console = nullptr;
	f32 BaseFontSize = 20;
	EFontUsage FontUsage;
	FColor8 BackgroundColor;
	bool bDrawShadowedText = false;

	KTimePoint LastAutoCompleteUpdateTime;
	TVector<u32> AutoCompleteHandles;

private:

	void DeleteRemovedLayouts(bool lock = true);

protected:

	void Draw() override;

	virtual void GetBackgroundBounds(KHudRectF& rect) = 0;
	virtual f32 GetMessageOpacity(class KTextMessage* msg) = 0;

	virtual void GetAutoSuggestions(TVector<KString>& vec, KTimePoint& time) {}
};

#endif