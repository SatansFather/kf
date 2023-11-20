#if !_SERVER && !_COMPILER

#include "hud_game_timer.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudGameTimer::KHudGameTimer()
{
	
}

void KHudGameTimer::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	u32 frameCount = iface->GetGameFrameCount();

	u32 seconds = frameCount / (1.0 / GameFrameDelta());
	u32 minutes = seconds / 60;
	seconds %= 60;

	KString m(minutes);
	KString s = (seconds < 10) ? KString("0" + KString(seconds)) : KString(seconds);

	if (seconds != PrevSeconds)
	{
		SecondsLayout = iface->HUD_CreateTextLayout(KString(":" + s), EFontUsage::GameTimer, 1000, 1000, SecondsLayout);
		PrevSeconds = seconds;
		SecondsWidth = iface->HUD_GetTextWidth(SecondsLayout);
	}

	if (minutes != PrevMinutes)
	{
		MinutesLayout = iface->HUD_CreateTextLayout(m, EFontUsage::GameTimer, 1000, 1000, MinutesLayout);
		PrevMinutes = minutes;
		MinutesWidth = iface->HUD_GetTextWidth(MinutesLayout);
	}

	const f32 scaledY = GetScaledY();
	const f32 viewX = GetViewportX();

	KHudPointF p;
	p.x = viewX / 2;
	p.y = scaledY * 3;

	//glm::vec2 proj = iface->ProjectWorldToScreen(FVec3(200, 600, 300).ToGLM());
	//p.x = proj.x;
	//p.y = proj.y;

	DrawTextShadowed(SecondsLayout, p, 3, FColor8(255, 255, 255, 255), FColor8(40, 40, 40, 255));
	p.x -= MinutesWidth;
	DrawTextShadowed(MinutesLayout, p, 3, FColor8(255, 255, 255, 255), FColor8(40, 40, 40, 255));
}

void KHudGameTimer::OnWindowResize()
{
	// force redraw text
	PrevMinutes = MAX_U32;
	PrevSeconds = MAX_U32;
}

#endif