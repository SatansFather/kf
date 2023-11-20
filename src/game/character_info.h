#pragma once

#include "kfglobal.h"
#include "entity/powerup_index.h"
#include "entity/weapon_index.h"

struct KCharacterWeaponInfo
{
	u16 Ammo;
	bool Has;
	bool Equip;
};

struct KCharacterInfo
{
	enum
	{
		SF_NewCharacter = 1,
		SF_Health       = 2,
		SF_Powerups     = 4,
		SF_Weapons      = 8,
	};


	u8 ShowFlags = 0;
	f32 DamageMultiplier = 1;
	i32 Health = 0;
	u16 PowerupFramesRemaining[EPowerupID::NumPowerups];
	KCharacterWeaponInfo Weapons[EWeaponID::NumWeapons];

	KCharacterInfo();

	void SetNewCharacter(bool n);
	void CopyFrom(const KCharacterInfo& info);
	void Reset();
};