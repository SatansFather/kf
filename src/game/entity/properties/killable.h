#pragma once

#include "../ent_prop.h"
#include "engine/net/snapshottable.h"

class KEntProp_Killable : public KEntProp
{
	enum KillableFlags
	{
		Damageable		= 1,
		Healable		= 2,
		Resurrectable	= 4,

		PickUpHP = 8
	};
public:
	SNAP_PROP(i16, Health = 100, SNAP_SEND_OWNER)
	u32 DamageTaken = 0;
	u32 HealingTaken = 0;
	u16 LifeCount = 1;
	u16 MaxLives = MAX_U16;
	u8 KillableFlags = Damageable;
	u8 LastDamagingPlayerIndex = NULL_PLAYER;
	u32 LastPainSoundFrame = 0;

protected:

	virtual void OnDamaged(bool dead, class KNetPlayer* player = nullptr) {};
	virtual void OnHealed(class KNetPlayer* player = nullptr) {};
	virtual void OnKilled(class KNetPlayer* player = nullptr);
	virtual void OnResurrected(class KNetPlayer* player = nullptr) {};

public:

	// returns true if the entity is still alive
	bool SetHealth(i32 health, bool triggerEvent = true, class KNetPlayer* player = nullptr);

	void TakeDamage(i32 damage, class KNetPlayer* player = nullptr);
	void TakeHeal(i32 heal, class KNetPlayer* player = nullptr);

	i32 GetHealth() const;
	bool IsDead() const { return Health <= 0; }

	u16 GetLifeCount() const { return LifeCount; }
	u16 GetMaxLives() const { return MaxLives; }
	u16 GetDeathCount() const { return LifeCount - 1; }

	bool CanBeDamaged();
	bool CanBeHealed() const { return KillableFlags & Healable; }
	bool CanBeResurrected() const { return (KillableFlags & Resurrectable) && LifeCount < MaxLives; }
	bool CanPickUpHealth() const { return KillableFlags & PickUpHP; }

	u8 GetLastDamagingPlayerIndex() const { return LastDamagingPlayerIndex; }

	void SetCanBeDamaged(bool v);
	void SetCanBeHealed(bool v);
	void SetCanBeResurrected(bool v);
	void SetCanPickUpHealth(bool v);
};