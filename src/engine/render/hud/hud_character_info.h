#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"
#include "hud_widget.h"
#include "game/character_info.h"

class KHudCharacterInfo : public KHudWidget
{
	friend class KRenderInterface;

	UPtr<class KHudBitmap> HealthIcon;
	UPtr<class KHudBitmap> HealthIconWarning;
	UPtr<class KHudBitmap> HealthBack;
	UPtr<class KHudBitmap> TextNoise;

	u32 HealthLayout = 0;
	KCharacterInfo Info;
	KCharacterInfo LastInfo;

	u32 WeaponTextHandles[EWeaponID::NumWeapons];
	u32 DamageMultiplierHandle = 0;

	f64 LastHealthUpdateTime = -1;
	f32 HealthWidth, HealthHeight;
	bool LastHealthDecrease = false;

public:

	KHudCharacterInfo();
	~KHudCharacterInfo();

private:

	void Draw() override;

	FColor8 GetWeaponColor(u32 index);
};

#endif