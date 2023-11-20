#if !_SERVER && !_COMPILER

#include "hud_target_name.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudTargetName::KHudTargetName()
{
	
}

KHudTargetName::~KHudTargetName()
{
	
}

void KHudTargetName::Draw()
{
	if (TargetPlayerName == "") return;
	KRenderInterface* iface = GetRenderInterface();
	if (LastTargetPlayerName.Get() != TargetPlayerName.Get())
		TextHandle = iface->HUD_CreateTextLayout(TargetPlayerName, EFontUsage::Console, 1000, 1000, TextHandle);

	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();
	const f32 centerX = viewX / 2;
	const f32 centerY = viewY / 2;
	const f32 scaleY = GetScaledY();
	const f32 texW = iface->HUD_GetTextWidth(TextHandle);

	KHudPointF p;
	p.x = centerX - texW / 2;
	p.y = centerY + scaleY * 32;

	DrawTextShadowed(TextHandle, p, scaleY, FColor8(200, 200, 200, 255), FColor8(40, 40, 40, 255));
	iface->HUD_DrawTextLayout(TextHandle, p);

	LastTargetPlayerName = TargetPlayerName;
}

#endif
