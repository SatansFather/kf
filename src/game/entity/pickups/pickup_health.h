#pragma once

#include "../pickup.h"
#include "../powerup.h"
#include "../properties/renderable.h"
#include "engine/net/snapshottable.h"

class KEntity_Pickup_Health :
	public KEntity_PickupBase
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"healthframe", "healthframe">, KHealthCrystal, KDynamicLight>
#endif
{
public:

	i32 HealValue = 50;
	u32 MaxHealth = 100;
#if !_SERVER
	f32 Scale = 1;
	FColor32 CrystalColor;
#endif

public:

	KEntity_Pickup_Health();

	bool CanPickUp(KEntity* ent) override;
	void PickUp(KEntity* ent) override;

	void OnRep_ReppedFlags() override;

	void SetHealthType(u8 type);
	static void SpawnDrop(u8 flagValue, const GVec3& postiion);

	KString GetPickupMessage() override;

	KSoundID GetRespawnSound() const override;
	KSoundID GetDespawnSound() const override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"healthframe", "healthframe">& frame, KHealthCrystal& crystal, KDynamicLight& light) override;
#endif
};