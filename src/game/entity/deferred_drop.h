#pragma once

#include "kfglobal.h"
#include "powerup_index.h"
#include "weapon_index.h"
#include "../../engine/math/vec3.h"

struct KDeferredDropSpawn
{
	enum 
	{
		Type_Health,
		Type_Powerup,
		Type_Weapon
	} ItemType;

	GVec3 Position;
	u8 HealthFlags;
	u32 FrameDuration = 0;
	u8 PowerupIndex = EPowerupID::NumPowerups;
	u8 WeaponIndex = EWeaponID::NumWeapons;
	void Spawn();
	void Finalize();
};
