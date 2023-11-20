#if !_SERVER && !_COMPILER

#include "hud_pickup_message.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudPickupMessage::KHudPickupMessage()
{

}

void KHudPickupMessage::Draw()
{
	const f64 screenTime = 4;
	
	if (KTime::Since(LastUpdateTime) > screenTime)
	{
		Text = "";
		LastDrawText = "";
		Combo = 0;
		return;
	}

	if (!Text.IsEmpty())
	{
		KString drawText = Text;
		if (Combo > 0) drawText += " x" + KString(Combo + 1);

		KRenderInterface* iface = GetRenderInterface();

		if (LastDrawText.Get() != drawText.Get())
			Handle = iface->HUD_CreateTextLayout(drawText, EFontUsage::FragMessage, 1000, 1000, Handle);

		KHudPointF p;
		p.x = GetViewportX() * .1;// - iface->HUD_GetTextWidth(Handle) * .5;
		p.y = GetViewportY() * .8;

		u8 opacity = 255;
		opacity *= 1 - KSaturate(KTime::Since(LastUpdateTime) - 3);

		DrawTextShadowed(Handle, p, GetScaledY(), FColor8(100, 200, 50, opacity), FColor8(40, 40, 40, opacity));

		LastDrawText = drawText;
	}
}

#endif