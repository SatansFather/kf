#pragma once

#include "entity.h"
#include "weapon_index.h"
#include "engine/net/snapshottable.h"

struct EWeaponCooldown
{
	enum Flags
	{
		None	= 0,
		Primary = 1,
		Alt		= 2,
		Both	= 3, // primary and alt can be ORd together for this
	};
};

enum class ECooldownSharing
{
	Shared,
	Unshared,
	PrimaryRequireAlt,
	AltRequirePrimary
};

class KEntity_Weapon : public KEntity, public KSnapshottable
{
	friend class KEntProp_WeaponInventory;
	
protected:

	struct KFireInputInfo
	{
		GVec3 Forward, Right, CamPosition;
		GFlt Pitch = 0, Yaw = 0;
		bool bIsMine = false;
		KEntity* Carrier = nullptr;
	};

protected:

	u8 WeaponID = EWeaponID::NumWeapons;
	u32 PrimaryFireCooldown = 0;
	u32 AltFireCooldown = 0;
	u8 CanSwitchOnCooldown = EWeaponCooldown::None;
	
private:

	u32 LastPrimaryFireFrame = 0;
	u32 LastAltFireFrame = 0;
	
protected:

	ECooldownSharing CooldownSharing = ECooldownSharing::Shared;

public:

	SNAP_PROP(u16, AmmoCount = 0)
	SNAP_PROP_TRANSIENT(u32, OwningEntityID = 0)
	void GetTransient_OwningEntityID(u32& val);
	void SetTransient_OwningEntityID(u32& val);

	TObjRef<KEntity> CarryingEntity;

	virtual bool Fire() = 0;
	virtual bool AltFire() = 0;

	virtual bool RequireAmmoForFire(bool primary) const { return true; }

	virtual void PrimaryFireCosmetics() {};
	virtual void AltFireCosmetics() {};

	bool Wep_CanFire(bool primary) const;

	void Tick() override;

	virtual u32 GetMaxAmmo() const { return 100; }
	virtual u32 GetRegenCount() const { return 1; }

protected:

	// client-spawned net objects
	void AddToCarryingInventory();

public:

	bool CanPrimaryFire() const;
	bool CanAltFire() const;

	u16 GetAmmoCount() const { return AmmoCount; }
	void SetAmmoCount(u16 ammo) { AmmoCount = ammo; }

	void ConsumeAmmo(u16 count = 1);
	void AddAmmo(u16 count);

	bool IsEquipped() const;

	KEntity* GetCarryingEntity() const;

	u8 GetCooldownState() const;
	u8 GetCanSwitchOnCooldown() const { return CanSwitchOnCooldown; }

	void ForceEndCooldown(u8 cooldown = EWeaponCooldown::Both);

#if !_SERVER
	bool UpdateWeaponRenderBuffer(struct KStaticMeshBase* entry);
#endif

protected:
	void GetInputInfo(KFireInputInfo& info);
};