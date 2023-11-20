#pragma once

#include "kfglobal.h"
#include "../weapon.h"
#include "../ent_prop.h"
#include "../weapon_index.h"

enum class EWeaponSwitchState : u8
{
	Holding,
	Dropping,
	Raising
};

class KEntProp_WeaponInventory : public KEntProp
{
	friend class KEntity_Weapon;

public:
	SNAP_PROP(u8, ReplicatedWeaponIndex = EWeaponID::Shotgun, SNAP_SEND_OTHERS);
	void OnRep_ReplicatedWeaponIndex();
private:
	u8 CurrentEquippedIndex = EWeaponID::Shotgun;
	u8 PendingWeaponIndex = EWeaponID::Shotgun;
	EWeaponSwitchState WeaponSwitchState = EWeaponSwitchState::Raising;
	u32 WeaponSwitchStartFrame = MAX_U32;
	f32 WeaponSwitchTime = .4;
	f32 WeaponSwitchAlpha = 0;
	bool bInitializingWeaponInventory = true;

	TObjRef<KEntity_Weapon> CarriedWeapons[EWeaponID::NumWeapons];

	u8 WepInv_ApplyWeaponScroll(i16 scroll);
	void WepInv_UpdateSwitch();
	void WepInv_SetWeaponSwitchStartFrame();

	void WepInv_TryFireWeapon(u8 state);

protected:

	// call on tick from owner
	void UpdateWeaponState(u8 inputState, u8 weaponIndex, i32 weaponScroll = 0);

public:

	KEntProp_WeaponInventory();
	~KEntProp_WeaponInventory();

	void SwitchToWeaponIndex(u8 index);
	void SwitchToWeaponObject(KEntity_Weapon* wep);
	bool HasWeaponIndex(u8 index) const;

	KEntity_Weapon* GetEquippedWeapon();
	KEntity_Weapon* GetPendingWeapon();
	KEntity_Weapon* GetWeaponByIndex(u8 index);
	u8 GetCurrentWeaponIndex() const { return CurrentEquippedIndex; }
	u8 GetPendingWeaponIndex() const { return PendingWeaponIndex; }

	void AddWeaponByID(u8 id, u16 ammo);

	void GetWeaponInfoArray(struct KCharacterWeaponInfo* arr);

	void DestroyAllWeapons();
};