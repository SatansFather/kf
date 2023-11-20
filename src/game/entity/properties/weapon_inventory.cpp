#include "weapon_inventory.h"
#include "engine/input/binding.h"
#include "../weapons/wep_shotgun.h"
#include "../weapons/wep_zapper.h"
#include "../weapons/wep_rocket.h"
#include "engine/game/match.h"

// DELETE
#include "../../../engine/game/local_player.h"
#include "../powerups/powerup_invis.h"
#include "../weapons/wep_cannon.h"
#include "../weapons/wep_blast.h"
#include "../../../engine/game_instance.h"
#include "../deferred_drop.h"

KEntProp_WeaponInventory::KEntProp_WeaponInventory()
{
	memset(CarriedWeapons, 0, sizeof(TObjRef<KEntity_Weapon>) * EWeaponID::NumWeapons);
}

KEntProp_WeaponInventory::~KEntProp_WeaponInventory()
{
	if (IsNetAuthority())
	{
		if (KGameInstance::Get().IsDestroyingMatch())
			return;

		if (KEntity_Weapon* w = GetEquippedWeapon())
		{
			KDeferredDropSpawn d;
			d.Position = w->GetPosition();
			d.ItemType = KDeferredDropSpawn::Type_Weapon;
			d.WeaponIndex = w->WeaponID;
			d.Finalize();
		}
	}

	DestroyAllWeapons();
}

TObjRef<KEntity_Weapon> CreateWeapon(u8 id)
{
	switch (id)
	{
		case EWeaponID::Shotgun:
			return TDataPool<KEntity_Weapon_Shotgun>::GetPool()->CreateNew();
		case EWeaponID::Rocket:
			return TDataPool<KEntity_Weapon_Rocket>::GetPool()->CreateNew();
		case EWeaponID::Zapper:
			return TDataPool<KEntity_Weapon_Zapper>::GetPool()->CreateNew();
		case EWeaponID::Cannon:
			return TDataPool<KEntity_Weapon_Cannon>::GetPool()->CreateNew();
		case EWeaponID::Blast:
			return TDataPool<KEntity_Weapon_Blast>::GetPool()->CreateNew();
	}
	return nullptr;
}

void KEntProp_WeaponInventory::UpdateWeaponState(u8 inputState, u8 weaponIndex, i32 weaponScroll /*= 0*/)
{	
	// weaponIndex == 0 means no input

	bool startSwitch = true;

	if (weaponIndex == 0 && weaponScroll != 0)
		// next/prev input, no direct input
		weaponIndex = WepInv_ApplyWeaponScroll(weaponScroll);	
	else if (weaponIndex > 0)
		// direct weapon input
		weaponIndex -= u8(EInputAction::Weapon0); // translate index to 0
	else
		// no weapon switch input
		startSwitch = false;

	if (startSwitch && weaponIndex != PendingWeaponIndex)
		SwitchToWeaponIndex(weaponIndex);

	WepInv_UpdateSwitch();

	// if we're not switching, we can fire
	if (WeaponSwitchState == EWeaponSwitchState::Holding)
		WepInv_TryFireWeapon(inputState);
}

bool KEntProp_WeaponInventory::HasWeaponIndex(u8 index) const
{
	return CarriedWeapons[index].IsValid();
}

KEntity_Weapon* KEntProp_WeaponInventory::GetEquippedWeapon()
{
	return CarriedWeapons[CurrentEquippedIndex].Get();
}

KEntity_Weapon* KEntProp_WeaponInventory::GetPendingWeapon()
{
	return CarriedWeapons[PendingWeaponIndex].Get();
}

KEntity_Weapon* KEntProp_WeaponInventory::GetWeaponByIndex(u8 index)
{
	return CarriedWeapons[index].Get();
}

void KEntProp_WeaponInventory::AddWeaponByID(u8 id, u16 ammo)
{
	if (!HasMatchAuthority()) return;
	if (!CarriedWeapons[id].IsValid()) CarriedWeapons[id] = CreateWeapon(id);
	KEntity_Weapon* wep = CarriedWeapons[id].Get();
	wep->CarryingEntity = GetEntity();
	wep->AddAmmo(ammo);

	if (KSnapshottable* snap = dynamic_cast<KSnapshottable*>(this))
	  if (KSnapshottable* wepSnap = dynamic_cast<KSnapshottable*>(wep))
		wepSnap->OwningPlayerIndex = snap->OwningPlayerIndex;
}

void KEntProp_WeaponInventory::GetWeaponInfoArray(struct KCharacterWeaponInfo* arr)
{
	for (u8 i = 0; i < EWeaponID::NumWeapons; i++)
	{
		if (KEntity_Weapon* wep = CarriedWeapons[i].Get())
		{
			arr[i].Has = true;
			arr[i].Ammo = wep->GetAmmoCount();
			arr[i].Equip = PendingWeaponIndex == i;
		}
		else
		{
			arr[i].Has = false;
			arr[i].Ammo = 0;
			arr[i].Equip = false;
		}
	}
}

void KEntProp_WeaponInventory::DestroyAllWeapons()
{
	for (i32 i = 0; i < EWeaponID::NumWeapons; i++)
	  if (KEntity_Weapon* wep = CarriedWeapons[i].Get())
		wep->DestroyEntity();
}

void KEntProp_WeaponInventory::OnRep_ReplicatedWeaponIndex()
{
	SwitchToWeaponIndex(ReplicatedWeaponIndex);
}

u8 KEntProp_WeaponInventory::WepInv_ApplyWeaponScroll(i16 scroll)
{
	scroll /= abs(scroll); // clamp to 1

	if (scroll == -1 && WeaponSwitchState == EWeaponSwitchState::Holding) 
		GetLocalPlayer()->WeaponRenderInfo.NegateSwitchDirection();

	const auto incScroll = [scroll](i16 start) -> i16
	{
		start += scroll;
		if (start < 0) start = EWeaponID::NumWeapons - 1;
		start %= EWeaponID::NumWeapons;
		return start;
	};

	for (i16 i = incScroll(PendingWeaponIndex); i != PendingWeaponIndex; i = incScroll(i))
	{
		if (HasWeaponIndex(i))
			return i;
	}

	return PendingWeaponIndex;
}

void KEntProp_WeaponInventory::WepInv_UpdateSwitch()
{
	// check cooldown
	if (KEntity_Weapon* wep = GetEquippedWeapon())
	{
		u8 state = wep->GetCooldownState();
		if (state)
		{
			// see if we can switch on cooldown
			u8 allowed = wep->GetCanSwitchOnCooldown();

			if (allowed != EWeaponCooldown::Both)
			{
				if (allowed != state)
				{
					WeaponSwitchStartFrame++;
					return;
				}
			}
		}
	}

	// able to switch at this point
	if (IsNetServer())
		ReplicatedWeaponIndex = PendingWeaponIndex;

	u32 framesToSwitch = KTime::FramesFromTime(WeaponSwitchTime);
	u32 frameCount = KTime::FramesSince(WeaponSwitchStartFrame);

	if (bInitializingWeaponInventory)
	{
		f32 age = GetEntity()->GetEntityAge();
		age /= WeaponSwitchTime;
		WeaponSwitchAlpha = .5 + age;
		if (WeaponSwitchAlpha >= 1) bInitializingWeaponInventory = false;
	}
	else if (WeaponSwitchStartFrame != MAX_U32)
		WeaponSwitchAlpha = f32(frameCount) / f32(framesToSwitch);

	if (WeaponSwitchAlpha > 1) WeaponSwitchAlpha = 1;

	if (GetViewedEntity() == GetEntity())
		GetLocalPlayer()->WeaponRenderInfo.SetWeaponSwitchAlpha(WeaponSwitchAlpha);

	if (WeaponSwitchAlpha >= 1)
	{
		WeaponSwitchState = EWeaponSwitchState::Holding;
		GetLocalPlayer()->WeaponRenderInfo.ResetSwitchDirection();
		return;
	}
	else if (WeaponSwitchAlpha < .5)
	{		
		WeaponSwitchState = EWeaponSwitchState::Dropping;
	}
	else // .5 <= alpha < 1
	{
		WeaponSwitchState = EWeaponSwitchState::Raising;
		CurrentEquippedIndex = PendingWeaponIndex;
	}
}

void KEntProp_WeaponInventory::WepInv_SetWeaponSwitchStartFrame()
{
	u32 frameCount = KTime::FrameCount();

	switch (WeaponSwitchState)
	{
		case EWeaponSwitchState::Holding:
		{
			WeaponSwitchStartFrame = frameCount;
			break;
		}
		case EWeaponSwitchState::Dropping:
		{
			if (PendingWeaponIndex != CurrentEquippedIndex)
				break;

			//GetLocalPlayer()->WeaponRenderInfo.NegateSwitchDirection();

			// continue to Raising
		}
		case EWeaponSwitchState::Raising:
		{
			WeaponSwitchStartFrame = frameCount - KTime::FramesFromTime(WeaponSwitchTime * (1 - WeaponSwitchAlpha));
			GetLocalPlayer()->WeaponRenderInfo.NegateSwitchDirection();
			break;
		}
	}
}

void KEntProp_WeaponInventory::WepInv_TryFireWeapon(u8 state)
{
	bool fired = false;
	if (KEntity_Weapon* wep = GetEquippedWeapon())
	{
		if (state & (u8)EInputKeyState::Firing)
		{
			if (wep->CanPrimaryFire() && wep->Fire())
			{
				fired = true;
				wep->PrimaryFireCosmetics();
				wep->LastPrimaryFireFrame = KTime::FrameCount();	
			}
		}
		else if (state & (u8)EInputKeyState::AltFiring)
		{
			if (wep->CanAltFire() && wep->AltFire())
			{
				fired = true;
				wep->AltFireCosmetics();
				wep->LastAltFireFrame = KTime::FrameCount();
			}
		}
	}

	if (fired)
	  if (KEntity_Powerup* powerup = GetEntity()->GetCarriedPowerup(EPowerupID::Invis))
	    dynamic_cast<KEntity_Powerup_Invis*>(powerup)->Break();
}

void KEntProp_WeaponInventory::SwitchToWeaponIndex(u8 index)
{
	index %= EWeaponID::NumWeapons;
	if (!HasWeaponIndex(index)) return;
	if (index == PendingWeaponIndex) return;
	bInitializingWeaponInventory = false;
	PendingWeaponIndex = index;
	WepInv_SetWeaponSwitchStartFrame();
}

void KEntProp_WeaponInventory::SwitchToWeaponObject(KEntity_Weapon* wep)
{
	
}
