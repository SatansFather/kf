#if !_SERVER && !_COMPILER
#include "hud_widget.h"
#include "../color.h"
#include "../interface/render_interface.h"

KHudWidget::KHudWidget()
{
	GetRenderInterface()->Widgets.push_back(this);
}

void KHudWidget::DrawTextShadowed(u32 handle, KHudPointF origin, f32 offset, FColor8 mainColor, FColor8 shadowColor, f32 scale)
{
	KRenderInterface* iface = GetRenderInterface();

	if (scale != 1)
	{
		const f32 scaledX = GetScaledX();
		const f32 scaledY = GetScaledY();
		const f32 viewX = GetViewportX();
		const f32 viewY = GetViewportY();

		const f32 baseSize = iface->HUD_GetFontSize(handle);
		const f32 fontSize = scale * baseSize;
		iface->HUD_SetTextLayoutFontSize(handle, fontSize * scaledY);

		f32 h = iface->HUD_GetTextHeight(handle);
		f32 w = iface->HUD_GetTextWidth(handle);

		const f32 healthHeight = h + scaledY * 3;

		const f32 wDiff = scale * w - w;
		const f32 hDiff = scale * healthHeight - healthHeight;

		origin.x -= wDiff / 2 - w - scaledY;
		origin.y -= hDiff / 2;
	}

	iface->HUD_SetDrawColor(shadowColor);
	origin.x += offset;
	origin.y += offset;
	iface->HUD_DrawTextLayout(handle, origin);

	iface->HUD_SetDrawColor(mainColor);
	origin.x -= offset;
	origin.y -= offset;
	iface->HUD_DrawTextLayout(handle, origin);
}

#endif