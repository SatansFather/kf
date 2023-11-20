#if !_SERVER && !_COMPILER

#include "hud_frame_stats.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/color.h"

KHudFrameStats::KHudFrameStats()
{
	OnWindowResize();
}

KHudFrameStats::~KHudFrameStats() {}

void KHudFrameStats::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	f32 scaledY = GetScaledY();
	f32 scaledX = GetScaledX();
	f32 viewY = GetViewportY();
	f32 viewX = GetViewportX();	

	KHudPointF point;
	point.x = scaledY;
	point.y = scaledY;

	
	UpdateStatHandle(iface->FrameStats.FrameRate, LastFrameRate, FrameRateValueHandle);
	UpdateStatHandle(iface->FrameStats.DrawCalls, LastDrawCalls, DrawCallValueHandle);
	UpdateStatHandle(iface->FrameStats.Triangles, LastTriangles, TriangleValueHandle);
	
	FColor8 mainColor = FColor8(255, 255, 150, 255);
	FColor8 shadowColor = FColor8(20, 20, 20, 255);

	u32 textHeight = iface->HUD_GetTextHeight(FrameRateHandle) + scaledY * 3;

	{	// descriptors
		DrawTextShadowed(FrameRateHandle, point, scaledY, mainColor, shadowColor);
		point.y += textHeight;
		DrawTextShadowed(DrawCallHandle, point, scaledY, mainColor, shadowColor);
		point.y += textHeight;
		DrawTextShadowed(TriangleHandle, point, scaledY, mainColor, shadowColor);
	}

	{	// values
		point.x += iface->HUD_GetTextWidth(FrameRateHandle);
		point.y = scaledY;

		DrawTextShadowed(FrameRateValueHandle, point, scaledY, mainColor, shadowColor);
		point.y += textHeight;
		DrawTextShadowed(DrawCallValueHandle, point, scaledY, mainColor, shadowColor);
		point.y += textHeight;
		DrawTextShadowed(TriangleValueHandle, point, scaledY, mainColor, shadowColor);
	}
}

void KHudFrameStats::OnWindowResize()
{
	KRenderInterface* iface = GetRenderInterface();

	FrameRateHandle = iface->HUD_CreateTextLayout("Frame Rate - ", EFontUsage::Stats, 1000, 1000);
	DrawCallHandle = iface->HUD_CreateTextLayout("Draw Calls - ", EFontUsage::Stats, 1000, 1000);
	TriangleHandle = iface->HUD_CreateTextLayout("Triangles  - ", EFontUsage::Stats, 1000, 1000);

	FrameRateValueHandle = iface->HUD_CreateTextLayout(LastFrameRate, EFontUsage::Stats, 1000, 1000);
	DrawCallValueHandle = iface->HUD_CreateTextLayout(LastDrawCalls, EFontUsage::Stats, 1000, 1000);
	TriangleValueHandle = iface->HUD_CreateTextLayout(LastTriangles, EFontUsage::Stats, 1000, 1000);
}

void KHudFrameStats::UpdateStatHandle(u32 current, u32& last, u32& handle)
{
	if (current != last)
	{
		handle = GetRenderInterface()->HUD_CreateTextLayout(current, EFontUsage::Stats, 1000, 1000, handle);
		last = current;
	}
}

#endif