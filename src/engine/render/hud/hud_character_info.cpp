#if !_SERVER && !_COMPILER

#include "hud_character_info.h"
#include "../interface/render_interface.h"
#include "../color.h"
#include "../interface/bitmap.h"

KHudCharacterInfo::KHudCharacterInfo()
{
	HealthIcon = GetRenderInterface()->HUD_LoadImage("healthicon");
	HealthIconWarning = GetRenderInterface()->HUD_LoadImage("healthicon_warning");
	HealthBack = GetRenderInterface()->HUD_LoadImage("healthback");
	TextNoise = GetRenderInterface()->HUD_LoadImage("textnoise");

	memset(WeaponTextHandles, 0, sizeof(u32) * EWeaponID::NumWeapons);
}

KHudCharacterInfo::~KHudCharacterInfo() {}

void KHudCharacterInfo::Draw()
{
	KRenderInterface* iface = GetRenderInterface();

	const f64 time = iface->GetTotalRenderTime();
	const f32 scaledX = GetScaledX();
	const f32 scaledY = GetScaledY();
	const f32 viewX = GetViewportX();
	const f32 viewY = GetViewportY();

	if (Info.ShowFlags & KCharacterInfo::SF_NewCharacter)
	{
		LastInfo.Health = 0;
	}

	if (Info.ShowFlags & KCharacterInfo::SF_Health)
	{
		if (LastInfo.Health != Info.Health || HealthLayout == 0)
		{
			HealthLayout = iface->HUD_CreateTextLayout(KString(Info.Health), EFontUsage::PowerupTimer, 1000, 1000, HealthLayout);

			bool lastDecrease = LastHealthDecrease;
			LastHealthDecrease = LastInfo.Health > Info.Health;
			if (time - LastHealthUpdateTime > .1 || lastDecrease != LastHealthDecrease)
				LastHealthUpdateTime = time;

			// find desired layout size at default font size
			iface->HUD_SetTextLayoutFontSize(HealthLayout, 45 * scaledY);
		}
		HealthHeight = iface->HUD_GetTextHeight(HealthLayout);
		HealthWidth = iface->HUD_GetTextWidth(HealthLayout);

		FColor8 healthColor(200, 255, 200, 255);
		const FColor8 decreaseColor(255, 0, 0, 255);
		const FColor8 increaseColor(0, 0, 255, 255);
		const f64 changeAnimTime = .25;
		const f64 timeSince = KClamp(time - LastHealthUpdateTime, 0, changeAnimTime) / changeAnimTime;

		healthColor = healthColor.Lerp(
			LastHealthDecrease ? decreaseColor : increaseColor, 
			1 - timeSince);

		KHudRectF back;
		back.left = .03 * viewX;
		back.right = .28 * viewX + HealthHeight + scaledY * 3;
		back.top = .85 * viewY;
		back.bottom = back.top + HealthHeight + scaledY * 3;
		//iface->HUD_SetDrawColor(FColor8(0, 0, 0, 150));
		//iface->HUD_FillRect(back);
		iface->HUD_DrawBitmap(HealthBack.get(), back.left, back.top, back.right - back.left, back.bottom - back.top);

		const f32 sizeAlpha = sin(2.f * PI<f32>() * timeSince + PI<f32>() / 2.f) / 2.f + .5;
		const f32 fontSize = KLerp(65, 45, sizeAlpha);
		iface->HUD_SetTextLayoutFontSize(HealthLayout, fontSize * scaledY);

		const f32 healthHeight = HealthHeight + scaledY * 3;

		const f32 fontScale = fontSize / 45;
		const f32 wDiff = fontScale * HealthWidth - HealthWidth;
		const f32 hDiff = fontScale * healthHeight - healthHeight;
		
		const f32 shakeScale = (1 - KSaturate((time - LastHealthUpdateTime) * 2)) * scaledY * 5;

		KHudPointF p;
		p.x = .2 * viewX;
		p.y = .85 * viewY;
		f32 iconScale = .9;
		if (LastHealthDecrease)
		{
			if (shakeScale > 0)
			{
				p.x += RandomSmoothValue(time * 20, shakeScale);
				p.y += RandomSmoothValue(2.5 + time * 20, shakeScale);
			}
		}
		else
		{
			iconScale = KLerp(1, iconScale, sizeAlpha);
			p.x += (healthHeight * (1 - iconScale) / 2);
		}

		if (Info.Health <= 50)
		{

			const f32 flashTime = time - (LastHealthUpdateTime);
			const f32 emergencyScale = KSaturate(1 - pow((f32)Info.Health / 50.f, 2));
			healthColor = healthColor.Lerp(FColor8(100, 125, 100, 255), emergencyScale);

			if (flashTime > 0)
			{
				const f32 flashRate = 1 + emergencyScale;
				const f32 flashAlpha = sin(2.f * PI<f32>() * (flashTime * pow(flashRate, 1.2)) + PI<f32>() / 2.f) / 2.f + .5;
				const FColor8 flashColor(200, 0, 0, 255);
				healthColor = healthColor.Lerp(flashColor, flashAlpha * KMax(.4, emergencyScale));

				if (!(LastHealthDecrease && timeSince < 1))
				{
					iconScale += flashAlpha * .05;
					p.x += (healthHeight * (1 - iconScale) / 2);
				}

				iface->HUD_DrawBitmap(HealthIcon.get(), p.x, p.y + (healthHeight * (1 - iconScale) / 2), healthHeight * iconScale, healthHeight * iconScale);
				iface->HUD_DrawBitmap(HealthIconWarning.get(), 
					p.x, p.y + (healthHeight * (1 - iconScale) / 2), 
					healthHeight * iconScale, 
					healthHeight * iconScale, 
					flashAlpha * emergencyScale * .5);
			}
		}
		else
		{
			iface->HUD_DrawBitmap(HealthIcon.get(), p.x, p.y + (healthHeight * (1 - iconScale) / 2), healthHeight * iconScale, healthHeight * iconScale);
		}

		p.x = .2 * viewX - wDiff / 2 - HealthWidth - scaledY * 10;
		p.y = .85 * viewY - hDiff / 2;
		if (LastHealthDecrease && shakeScale > 0)
		{
			p.x += RandomSmoothValue(2 + time * 20, shakeScale * 2);
			p.y += RandomSmoothValue(3 + time * 20, shakeScale * 2);
		}
		DrawTextShadowed(HealthLayout, p, scaledY * 3, healthColor, FColor8(40, 40, 40, 255));
		iface->HUD_SetBitmapBrush(TextNoise.get(), p.x, p.y, 1, 1, 1, true);
		iface->HUD_DrawTextLayout(HealthLayout, p);
	}
	if (Info.ShowFlags & KCharacterInfo::SF_Weapons)
	{
		KHudPointF p;
		p.x = .02 * viewX;
		p.y = .5 * viewY;

		for (u32 i = 0; i < EWeaponID::NumWeapons; i++)
		{
			KCharacterWeaponInfo& wep = Info.Weapons[i];
			KCharacterWeaponInfo& lastWep = LastInfo.Weapons[i];

			FColor8 wepColor = GetWeaponColor(i);

			if (wep.Has)
			{
				if (wep.Ammo != lastWep.Ammo || WeaponTextHandles[i] == 0)
					WeaponTextHandles[i] = iface->HUD_CreateTextLayout(KString(wep.Ammo), EFontUsage::PowerupTimer, 1000, 1000, WeaponTextHandles[i]);

				if (wep.Equip)
				{
					KHudRectF r;
					r.left = p.x - scaledY * 12;
					r.top = p.y;
					r.right = r.left + viewY * .1;
					r.bottom = r.top + iface->HUD_GetTextHeight(WeaponTextHandles[i]);
					iface->HUD_SetDrawColor(FColor8(255, 255, 255, 100));
					iface->HUD_FillRect(r);
					iface->HUD_SetDrawColor(FColor8(100, 100, 100, 200));
					iface->HUD_DrawRect(r, scaledY * 3);
				}

				DrawTextShadowed(WeaponTextHandles[i], p, scaledY * 3, wepColor, FColor8(40, 40, 40, 255));
			}

			p.y += iface->HUD_GetTextHeight(WeaponTextHandles[i]);
		}
	}
	//if (Info.DamageMultiplier != 1)
	{
		if (DamageMultiplierHandle == 0 || Info.DamageMultiplier != LastInfo.DamageMultiplier)
			DamageMultiplierHandle = iface->HUD_CreateTextLayout(KString(Info.DamageMultiplier, 2) + "x", EFontUsage::FragMessage, 1000, 1000, DamageMultiplierHandle);

		KHudPointF p;
		p.x = viewX * .18;
		p.y = viewY * .91;
		DrawTextShadowed(DamageMultiplierHandle, p, scaledY, FColor8(200, 255, 200, 255), FColor8(40, 40, 40, 255));
	}
	
	LastInfo.CopyFrom(Info);
}

FColor8 KHudCharacterInfo::GetWeaponColor(u32 index)
{
	switch (index)
	{
		case EWeaponID::Shotgun:
			return FColor8(255, 150, 0, 255);
		case EWeaponID::Rocket:
			return FColor8(255, 0, 0, 255);
		case EWeaponID::Cannon:
			return FColor8(143, 172, 117, 255);
		case EWeaponID::Blast:
			return FColor8(60, 60, 255, 255);
		case EWeaponID::Zapper:
			return FColor8(100, 222, 255, 255);
	}

	return FColor8(0, 0, 0, 0);
}

#endif