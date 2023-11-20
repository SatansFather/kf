#pragma once

#include "../pickup.h"
#include "../properties/renderable.h"
#include "engine/net/snapshottable.h"
#include "../weapon_index.h"

class KEntity_Pickup_Weapon :
	public KEntity_PickupBase
{
public:

	KEntity_Pickup_Weapon();

	bool CanPickUp(KEntity* ent) override;
	void PickUp(KEntity* ent) override;

	virtual u32 GetWeaponID() = 0;
	
	static void SpawnDrop(u8 id, const GVec3& postiion);

	KSoundID GetDespawnSound() const { return KSoundID::Pickup_Gun; }
	KSoundID GetRespawnSound() const { return KSoundID::Respawn_Weapon; }

	virtual u32 GetAmmoCount() const { return 10; }

#if !_SERVER
	bool UpdateRenderBuffer(KStaticMeshBase* entry, const GVec3& lastRenderPos);
#endif
};

class KEntity_Pickup_Weapon_Rocket :  public KEntity_Pickup_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "rockettex">>
#endif
{
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "rockettex">& entry) override;
#endif
	KString GetPickupMessage() override { return "Rocket Launcher"; }
	u32 GetWeaponID() override { return EWeaponID::Rocket; }
};

class KEntity_Pickup_Weapon_Cannon : public KEntity_Pickup_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "cannontex">>
#endif
{
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "cannontex">& entry) override;
#endif
	KString GetPickupMessage() override { return "Handcannon"; }
	u32 GetWeaponID() override { return EWeaponID::Cannon; }
	u32 GetAmmoCount() const override { return 20; }
};

class KEntity_Pickup_Weapon_Shotgun: public KEntity_Pickup_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "shotguntex">>
#endif
{
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "shotguntex">& entry) override;
#endif
	KString GetPickupMessage() override { return "Shotgun"; }
	u32 GetWeaponID() override { return EWeaponID::Shotgun; }
};

class KEntity_Pickup_Weapon_Blast : public KEntity_Pickup_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "blastertex">>
#endif
{
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "blastertex">& entry) override;
#endif
	KString GetPickupMessage() override { return "Blaster"; }
	u32 GetWeaponID() override { return EWeaponID::Blast; }
	u32 GetAmmoCount() const override { return 100; }
};