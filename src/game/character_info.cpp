#include "character_info.h"

KCharacterInfo::KCharacterInfo()
{
	memset(PowerupFramesRemaining, 0, sizeof(u16) * EPowerupID::NumPowerups);
}

void KCharacterInfo::SetNewCharacter(bool n)
{
	if (n) ShowFlags |= SF_NewCharacter;
	else   ShowFlags &= ~SF_NewCharacter;
}

void KCharacterInfo::CopyFrom(const KCharacterInfo& info)
{
	ShowFlags = info.ShowFlags;
	Health = info.Health;
	DamageMultiplier = info.DamageMultiplier;
	memcpy(PowerupFramesRemaining, info.PowerupFramesRemaining, EPowerupID::NumPowerups * sizeof(u16));
	memcpy(Weapons, info.Weapons, EWeaponID::NumWeapons * sizeof(KCharacterWeaponInfo));
}

void KCharacterInfo::Reset()
{
	memset(this, 0, sizeof(this));
}
