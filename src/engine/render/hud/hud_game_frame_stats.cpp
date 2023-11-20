#if !_SERVER && !_COMPILER

#include "hud_game_frame_stats.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/color.h"

KHudGameFrameStats::KHudGameFrameStats()
{
	memset(&LastTickTime, 0, sizeof(f64) * TICK_FRAMES_TRACKED);
	memset(&LastNetTime, 0, sizeof(f64) * TICK_FRAMES_TRACKED);
	memset(&LastRenderUpdateTime, 0, sizeof(f64) * TICK_FRAMES_TRACKED);
	memset(&LastCopyTime, 0, sizeof(f64) * TICK_FRAMES_TRACKED);
}

KHudGameFrameStats::~KHudGameFrameStats()
{

}

void KHudGameFrameStats::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	f32 scaledY = GetScaledY();
	f32 scaledX = GetScaledX();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();
	f32 pxWidth = scaledY * 200;
	f64 step = GameFrameDelta() / iface->GetGameTimeDilation();

	f64 tick = 0;
	for (u8 i = 0; i < TICK_FRAMES_TRACKED; i++) tick += LastTickTime[i];
	tick /= TICK_FRAMES_TRACKED;

	f64 net = 0;
	for (u8 i = 0; i < TICK_FRAMES_TRACKED; i++) net += LastNetTime[i];
	net /= TICK_FRAMES_TRACKED;

	f64 copy = 0;
	for (u8 i = 0; i < TICK_FRAMES_TRACKED; i++) copy += LastCopyTime[i];
	copy /= TICK_FRAMES_TRACKED;

	f64 render = 0;
	for (u8 i = 0; i < TICK_FRAMES_TRACKED; i++) render += LastRenderUpdateTime[i];
	render /= TICK_FRAMES_TRACKED;

	KHudRectF rect;
	rect.top = 100 * scaledY;
	rect.bottom = rect.top + 15 * scaledY;

	rect.left = scaledY * 5;
	rect.right = rect.left + pxWidth;
	iface->HUD_SetDrawColor(FColor32(.2, .2, .2, .5));
	iface->HUD_FillRect(rect);

	rect.left = scaledY * 5;
	rect.right = rect.left + (tick / step) * pxWidth;
	iface->HUD_SetDrawColor(FColor32(0, 1, 0, 1));
	iface->HUD_FillRect(rect);
	rect.left = rect.right;

	rect.right = rect.left + (net / step) * pxWidth;
	iface->HUD_SetDrawColor(FColor32(1, 0, 0, 1));
	iface->HUD_FillRect(rect);
	rect.left = rect.right;

	rect.right = rect.left + (render / step) * pxWidth;
	iface->HUD_SetDrawColor(FColor32(0, 0, 1, 1));
	iface->HUD_FillRect(rect);
	rect.left = rect.right;

	rect.right = rect.left + (copy / step) * pxWidth;
	iface->HUD_SetDrawColor(FColor8(1, 1, 1, 1));
	iface->HUD_FillRect(rect);
	rect.left = rect.right;

	if (KTime::Since(LastTextUpdateTime) > .25 || TextHandle == 0)
	{
		LastTextUpdateTime = KTime::Now();

		u32 count = KTime::FrameCount() % TICK_FRAMES_TRACKED;
		f64 total = tick + net + render + copy;
		TextHandle = iface->HUD_CreateTextLayout(KString(total * 1000, 2) + "ms", EFontUsage::Stats, 1000, 1000, TextHandle);
	}

	u32 width = iface->HUD_GetTextWidth(TextHandle);
	KHudPointF point;
	point.x = scaledY * 5 + pxWidth / 2 - width / 2;
	point.y = rect.top - scaledY * 2.2;
	DrawTextShadowed(TextHandle, point, scaledY * 2, FColor8(200, 200, 200, 255), FColor8(40, 40, 40, 255));
}

void KHudGameFrameStats::OnWindowResize()
{

}

void KHudGameFrameStats::AddTickTime(f64 time)
{
	u32 count = KTime::FrameCount() % TICK_FRAMES_TRACKED;
	LastTickTime[count] = time;
}

void KHudGameFrameStats::AddNetTime(f64 time)
{
	u32 count = KTime::FrameCount() % TICK_FRAMES_TRACKED;
	LastNetTime[count] = time;
}

void KHudGameFrameStats::AddCopyTime(f64 time)
{
	u32 count = KTime::FrameCount() % TICK_FRAMES_TRACKED;
	LastCopyTime[count] = time;
}

void KHudGameFrameStats::AddRenderUpdateTime(f64 time)
{
	u32 count = KTime::FrameCount() % TICK_FRAMES_TRACKED;
	LastRenderUpdateTime[count] = time;
}

#endif