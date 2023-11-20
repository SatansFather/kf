#if !_SERVER && !_COMPILER
#include "hud_frag_message.h"
#include "engine/render/interface/render_interface.h"
#include "../color.h"

KHudFragMessage::KHudFragMessage()
{
	KRenderInterface* iface = GetRenderInterface();
	YouFraggedLayout = iface->HUD_CreateTextLayout("You Fragged ", EFontUsage::FragMessage, 1000, 1000);
	FraggedByLayout = iface->HUD_CreateTextLayout("Fragged By ", EFontUsage::DeathMessage, 1000, 1000);
}

void KHudFragMessage::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	
	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();
	const f32 scaleY = GetScaledY();

	if (!CurrentFragMessage.PlayerName.IsEmpty())
	{
		f32 age = iface->GetTotalRenderTime() - CurrentFragMessage.TimeCreated;
		if (age < 4)
		{
			if (CurrentFragMessage.bUpdated || VictimLayout == 0)
				VictimLayout = iface->HUD_CreateTextLayout(CurrentFragMessage.PlayerName, EFontUsage::FragMessage, 1000, 1000, VictimLayout);

			const f32 fragW = iface->HUD_GetTextWidth(YouFraggedLayout);
			const f32 nameW = iface->HUD_GetTextWidth(VictimLayout);
			const f32 totalW = fragW + nameW;

			KHudPointF p;
			p.x = (GetViewportX() / 2) - (totalW / 2); 
			p.y = GetViewportY() * .3;

			f32 opacity = 1;
			if (age < .2) opacity *= age * 5;
			if (age > 3) opacity = 1 - (age - 3);
			opacity *= 255;

			f32 brightness = 1;
			if (age < .25)
				brightness += (sin((2 * 3.141592 * age * 4) - (3.141592 / 2.f)) / 2 + .5) * .5;
			
			DrawTextShadowed(
				YouFraggedLayout, p, scaleY * 2, 
				FColor32(.8 * brightness, .75 * brightness, .75 * brightness, opacity/255).To8(), 
				FColor8(40, 40, 40, opacity));

			p.x += fragW;

			DrawTextShadowed(
				VictimLayout, p, scaleY * 2,
				FColor32(.8 * brightness, 1 * brightness, .8 * brightness, opacity / 255).To8(),
				FColor8(40, 40, 40, opacity));
		}
	}

	if (!CurrentDeathMessage.PlayerName.IsEmpty() && CurrentDeathMessage.bShouldShow)
	{
		f32 age = iface->GetTotalRenderTime() - CurrentDeathMessage.TimeCreated;

		if (CurrentDeathMessage.bUpdated || KillerLayout == 0)
			KillerLayout = iface->HUD_CreateTextLayout(CurrentDeathMessage.PlayerName, EFontUsage::DeathMessage, 1000, 1000, KillerLayout);

		const f32 fragW = iface->HUD_GetTextWidth(FraggedByLayout);
		const f32 nameW = iface->HUD_GetTextWidth(KillerLayout);
		const f32 totalW = fragW + nameW;

		KHudPointF p;
		p.x = (GetViewportX() / 2) - (totalW / 2);
		p.y = GetViewportY() * .2;

		f32 brightness = 1;
		if (age < .5)
			brightness += (sin((2 * 3.141592 * age * 2) - (3.141592 / 2.f)) / 2 + .5) * .5;

		DrawTextShadowed(FraggedByLayout, p, scaleY * 2, FColor32(.8 * brightness, .75 * brightness, .75 * brightness, 1).To8(), FColor8(40, 40, 40, 255));
		p.x += fragW;
		DrawTextShadowed(KillerLayout, p, scaleY * 2, FColor32(.8 * brightness, 1 * brightness, .8 * brightness, 1).To8(), FColor8(40, 40, 40, 255));

	}
	

	CurrentDeathMessage.bUpdated = false;
	CurrentFragMessage.bUpdated = false;
}

#endif