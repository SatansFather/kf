#if !_SERVER && !_COMPILER

#include "hud_powerup_timers.h"
#include "../interface/render_interface.h"
#include "../interface/bitmap.h"
#include "../color.h"

KHudPowerupTimers::KHudPowerupTimers()
{
	PowerupIcons[EPowerupID::Brain] = GetRenderInterface()->HUD_LoadImage("game/brainicon");
	PowerupIcons[EPowerupID::Invis] = GetRenderInterface()->HUD_LoadImage("game/invisicon");
	PowerupNoise = GetRenderInterface()->HUD_LoadImage("powerupnoise");
	PowerupGlow = GetRenderInterface()->HUD_LoadImage("powerupglow");
	Reset();
}

void KHudPowerupTimers::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	if (iface->IsLoadingMap())
	{
		Reset();
		return;
	}

	f64 time = iface->GetTotalRenderTime();
	
	f32 scaledX = GetScaledX();
	f32 scaledY = GetScaledY();
	f32 viewX = GetViewportX();
	f32 viewY = GetViewportY();

	f32 centerX = .8 * viewX;
	f32 centerY = .8 * viewY;
	f32 radius = scaledY * 45;
	for (u8 i = 0; i < EPowerupID::NumPowerups; i++)
	{
		f64 timeSince = time - LastPowerupUpdateTimes[i];
		if (FramesRemaining[i] > 0 || timeSince < .5 && LastPowerupUpdateTimes[i] > 0)
		{
			const u32 remaining = FramesRemaining[i];
			f32 alpha = 1;
			
			if (remaining == 0)
				alpha = LerpFade(1 - timeSince * 2);


			const f32 glowScale = 11 + sin(2 * time + i);
			iface->HUD_DrawBitmap(PowerupGlow.get(), 
				centerX - radius - scaledY * glowScale, 
				centerY - radius - scaledY * glowScale, 
				radius * 2 + scaledY * glowScale * 2, 
				radius * 2 + scaledY * glowScale * 2, 
				alpha);

			iface->HUD_SetDrawColor(FColor32(0, 0, 0, alpha));
			iface->HUD_FillEllipse(centerX, centerY, radius * .8, radius * .8);

			f32 picSize = 90 * scaledY + (sin(time) * 5);
			f32 picX = centerX;
			f32 picY = centerY;

			picX += sin(time * 1.2) * 3;
			picY += cos(time * .9) * 3;

			f32 w = PowerupIcons[i]->GetWidth();
			f32 h = PowerupIcons[i]->GetHeight();

			iface->HUD_SetBitmapBrush(PowerupIcons[i].get(), picX - (w * picSize / w / 2), picY - (h * picSize / h / 2), picSize / w, picSize / h, alpha);
			iface->HUD_FillEllipse(centerX, centerY, radius * .7, radius * .7);

			iface->HUD_SetRadialBrushForPowerup(EPowerupID::NumPowerups + 1, std::pow(alpha, .5));
			iface->HUD_FillEllipse(centerX, centerY, radius * .8, radius * .8);

			f32 progress = f32(remaining) / (60.f * 30.f);

			iface->HUD_SetRadialBrushForPowerup(EPowerupID::NumPowerups, std::pow(alpha, .5));
			iface->HUD_DrawCircleProgressBar(centerX, centerY, radius, 1);

			iface->HUD_SetRadialBrushForPowerup(i, std::pow(alpha, .5));
			iface->HUD_DrawCircleProgressBar(centerX, centerY, radius, progress);	

			if (remaining > 0)
			{
				KString seconds = (i32)std::ceil((f32)remaining * GameFrameDelta());
				u32 handle = iface->HUD_CreateTextLayout(seconds, EFontUsage::PowerupTimer, 1, 1);
				w = iface->HUD_GetTextWidth(handle) / 2;
				h = iface->HUD_GetTextHeight(handle) / 2;

				f32 fontBright = 1;
				f32 tick = ((remaining) % 60);
				f32 alpha = sin(2 * PI<f32>() * ( KSaturate(tick / 10) * .75) ) / 2.f + .5;
				fontBright = KLerp(fontBright, .8, alpha);

				const f32 textAlpha = KSaturate((f32)remaining / 30.f) * 255;
				DrawTextShadowed(
					handle, 
					{ centerX - w, centerY - h }, 
					scaledY * 2, 
					FColor8(200 * fontBright, 255 * fontBright, 200 * fontBright, textAlpha), 
					FColor8(40, 40, 40, textAlpha));
			}

			iface->HUD_DrawBitmap(PowerupNoise.get(), centerX - radius, centerY - radius, radius * 2, radius * 2, 1);

			centerX -= radius * 2 + scaledY;

			if (FramesRemaining[i] > 0)
				LastPowerupUpdateTimes[i] = time;
		}
	}
}

void KHudPowerupTimers::OnWindowResize()
{
	
}

void KHudPowerupTimers::Reset()
{
	memset(FramesRemaining, 0, sizeof(u16) * EPowerupID::NumPowerups);
	memset(LastPowerupUpdateTimes, 0, sizeof(f64) * EPowerupID::NumPowerups);
}

#endif