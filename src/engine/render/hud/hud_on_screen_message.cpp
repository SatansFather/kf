#if !_SERVER && !_COMPILER

#include "hud_on_screen_message.h"
#include "game/on_screen_message.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudOnScreenMessages::KHudOnScreenMessages()
{
	
}

void KHudOnScreenMessages::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	for (i32 i = Messages.size() - 1; i >= 0; i--)
	{
		KOnScreenMessage& m  = Messages[i];
		m.Duration -= iface->GetLastFrameTime();

		if (m.Duration <= 0)
		{
			VectorRemoveAt(Messages, i);
			continue;
		}

		if (m.TextLayout == 0)
			m.TextLayout = iface->HUD_CreateTextLayout(m.Text, EFontUsage::OnScreenMessage, 1000, 1000);
		
		KHudPointF point;
		point.x = GetViewportX() / 2 - (iface->HUD_GetTextWidth(m.TextLayout) / 2);
		point.y = GetViewportY() * .15 + (iface->HUD_GetTextHeight(m.TextLayout) * m.Slot);

		f32 a = KSaturate(m.Duration);

		DrawTextShadowed(m.TextLayout, point, GetScaledY(), FColor8(200, 200, 200, 255 * a), FColor8(40, 40, 40, 255 * a));
	}
}

void KHudOnScreenMessages::SubmitPending(TVector<KOnScreenMessage>& pending)
{
	for (KOnScreenMessage& m : pending)
	{
		bool overwrite = false;
		for (u32 i = 0; i < Messages.size(); i++)
		{
			if (Messages[i].Slot == m.Slot)
			{
				overwrite = true;
				Messages[i] = m;
				break;
			}
		}
		if (!overwrite)
		{
			Messages.push_back(m);
		}
	}
	pending.clear();
}

#endif
