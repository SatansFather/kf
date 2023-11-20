#if !_SERVER && !_COMPILER

#include "hud_kill_feed.h"
#include "../interface/render_interface.h"
#include "../color.h"

KHudKillFeed::KHudKillFeed()
{
	KRenderInterface* iface = GetRenderInterface();

	PreVictimHandles.push_back(iface->HUD_CreateTextLayout(" fragged ", EFontUsage::KillFeed, 1000, 1000));
	PostVictimHandles.push_back(iface->HUD_CreateTextLayout("", EFontUsage::KillFeed, 1000, 1000));

	PreVictimHandles.push_back(iface->HUD_CreateTextLayout("", EFontUsage::KillFeed, 1000, 1000));
	PostVictimHandles.push_back(iface->HUD_CreateTextLayout(" died", EFontUsage::KillFeed, 1000, 1000));
}

void KHudKillFeed::Draw()
{
	if (KillFeedMessages.size() == 0) return;

	KRenderInterface* iface = GetRenderInterface();

	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();
	const f32 scaleY = GetScaledY();

	f32 textHeight = iface->HUD_GetTextHeight(PreVictimHandles[0]);

	KHudPointF p;
	p.x = 0;
	p.y = textHeight * MaxOnScreenCount * scaleY;

	bool erasing = false;

	for (i32 i = KillFeedMessages.size() - 1; i >= 0; i--)
	{
		if (erasing) VectorRemoveAt(KillFeedMessages, i);
		if (KillFeedMessages.size() == 0) break;
		if (i > KillFeedMessages.size() - 1) continue;

		const KKillFeedMessage& message = KillFeedMessages[i];
		const f32 age = iface->GetTotalRenderTime() - message.TimeAdded;
		if (age >= 5)
		{
			erasing = true;
			VectorRemoveAt(KillFeedMessages, i);
			continue;
		}

		u32 victimLayout = 0;
		u32 killerLayout = 0;

		f32 killerNameW = 0;
		f32 victimNameW = 0;
		f32 preW = iface->HUD_GetTextWidth(PreVictimHandles[message.DamageType]);
		f32 postW = iface->HUD_GetTextWidth(PostVictimHandles[message.DamageType]);

		LayoutHandle& victimRef = PlayerNameLayouts[message.VictimName.Get()];
		if (victimRef.Handle == 0)
			victimRef.Handle = iface->HUD_CreateTextLayout(message.VictimName, EFontUsage::KillFeed, 1000, 1000);
		victimLayout = victimRef.Handle;
		victimNameW = iface->HUD_GetTextWidth(victimLayout);

		if (!message.KillerName.IsEmpty())
		{
			LayoutHandle& killerRef = PlayerNameLayouts[message.KillerName.Get()];
			if (killerRef.Handle == 0)
				killerRef.Handle = iface->HUD_CreateTextLayout(message.KillerName, EFontUsage::KillFeed, 1000, 1000);
			killerLayout = killerRef.Handle;
			killerNameW = iface->HUD_GetTextWidth(killerLayout);
		}

		const f32 totalW = killerNameW + victimNameW + preW + postW;

		f32 opacity = 1;
		if (age < .2)
		{
			opacity = age * 5;
			p.y += pow(KSaturate(1 - opacity), 2) * textHeight;
		}
		else if (age > 4) opacity = 5 - age;

		FColor8 textColor(200, 200, 200, KSaturate(opacity) * 255);
		FColor8 nameColor(200, 255, 200, KSaturate(opacity) * 255);
		FColor8 backColor(40, 40, 40, KSaturate(opacity) * 255);

								// padding
		p.x = viewX - totalW - (scaleY * 3);
		
		if (killerLayout != 0)
		{
			DrawTextShadowed(killerLayout, p, scaleY * 2, nameColor, backColor);
			p.x += killerNameW;
		}
		
		if (PreVictimHandles[message.DamageType] != 0)
		{
			DrawTextShadowed(PreVictimHandles[message.DamageType], p, scaleY * 2, textColor, backColor);
			p.x += preW;
		}

		DrawTextShadowed(victimLayout, p, scaleY * 2, nameColor, backColor);
		p.x += victimNameW;
		DrawTextShadowed(PostVictimHandles[message.DamageType], p, scaleY * 2, textColor, backColor);

		p.y -= textHeight;
		if (p.y < -textHeight * 2) erasing = true;
	}
}

#endif