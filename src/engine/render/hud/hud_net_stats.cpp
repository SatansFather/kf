#if !_SERVER && !_COMPILER

#include "hud_net_stats.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/color.h"

KHudNetStats::KHudNetStats()
{
	OnWindowResize();
}

KHudNetStats::~KHudNetStats() {}

void KHudNetStats::DrawBackground()
{
	KRenderInterface* iface = GetRenderInterface();

	f32 scaledY = GetScaledY();
	f32 scaledX = GetScaledX();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();

	KHudRectF background;
	background.top = Top + scaledY * 1.5;
	background.bottom = Bottom + scaledY * 1.5;
	background.left = Left + scaledY * 1.5;
	background.right = Right + scaledY * 1.5;
	iface->HUD_SetDrawColor(FColor8(20, 20, 20, 150));
	iface->HUD_FillRect(background);

	background.top -= scaledY * 1.5;
	background.bottom -= scaledY * 1.5;
	background.left -= scaledY * 1.5;
	background.right -= scaledY * 1.5;
	iface->HUD_SetDrawColor(FColor8(40, 100, 100, 150));
	iface->HUD_FillRect(background);

}

void KHudNetStats::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	f32 scaledY = GetScaledY();
	f32 scaledX = GetScaledX();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();

	Top = viewY * .18;
	Bottom = viewY * .18 + scaledY * 150;
	Left = viewX * .01;
	Right = viewX * .01 + scaledY * 200;

	//DrawBackground();
	
	KHudPointF p;
	p.x = Left;
	p.y = Top;

	DrawTextShadowed(FrameDiffHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));

	FrameDiffValueHandle = iface->HUD_CreateTextLayout(KString(NetStats.ClientFrameDiff, 3), EFontUsage::Stats, 1, 0, FrameDiffValueHandle);
	f32 w = iface->HUD_GetTextWidth(FrameDiffValueHandle);
	p.x = Right - w;
	DrawTextShadowed(FrameDiffValueHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));
	
	p.x = Left;
	f32 h = iface->HUD_GetTextHeight(FrameDiffHandle);
	p.y += h;
	DrawTextShadowed(TimeDilationHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));

	u32 timeScale = NetStats.TimeDilation * 100;
	TimeDilationValueHandle = iface->HUD_CreateTextLayout(timeScale, EFontUsage::Stats, 1, 0, TimeDilationValueHandle);
	w = iface->HUD_GetTextWidth(TimeDilationValueHandle);
	p.x = Right - w;
	DrawTextShadowed(TimeDilationValueHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));

	p.y += h;
	p.x = Left;
	DrawTextShadowed(ActualDelayHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));
	u32 delay = std::round(NetStats.ActualDelay * 1000);
	ActualDelayValueHandle = iface->HUD_CreateTextLayout(KString(delay) + "ms", EFontUsage::Stats, 1, 0, ActualDelayValueHandle);
	w = iface->HUD_GetTextWidth(ActualDelayValueHandle);
	p.x = Right - w;
	DrawTextShadowed(ActualDelayValueHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));

	p.y += h;
	p.x = Left;
	DrawTextShadowed(InBytesHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));
	InBytesValueHandle = iface->HUD_CreateTextLayout(NetStats.InBytes, EFontUsage::Stats, 1, 0, InBytesValueHandle);
	w = iface->HUD_GetTextWidth(InBytesValueHandle);
	p.x = Right - w;
	DrawTextShadowed(InBytesValueHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));

	p.y += h;
	p.x = Left;
	DrawTextShadowed(OutBytesHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));
	OutBytesValueHandle = iface->HUD_CreateTextLayout(NetStats.OutBytes, EFontUsage::Stats, 1, 0, OutBytesValueHandle);
	w = iface->HUD_GetTextWidth(OutBytesValueHandle);
	p.x = Right - w;
	DrawTextShadowed(OutBytesValueHandle, p, scaledY, FColor8(200, 200, 200, 120), FColor8(30, 30, 30, 120));


	LastNetStats = NetStats;
}

void KHudNetStats::OnWindowResize()
{
	FrameDiffHandle = GetRenderInterface()->HUD_CreateTextLayout("Frame Diff", EFontUsage::Stats, 1);
	TimeDilationHandle = GetRenderInterface()->HUD_CreateTextLayout("Time", EFontUsage::Stats, 1);
	ActualDelayHandle = GetRenderInterface()->HUD_CreateTextLayout("Actual Delay", EFontUsage::Stats, 1);
	InBytesHandle = GetRenderInterface()->HUD_CreateTextLayout("In Bytes/s", EFontUsage::Stats, 1);
	OutBytesHandle = GetRenderInterface()->HUD_CreateTextLayout("Out Bytes/s", EFontUsage::Stats, 1);
}

#endif
