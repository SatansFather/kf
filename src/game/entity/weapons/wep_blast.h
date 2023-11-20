#pragma once

#include "../weapon.h"
#include "../properties/renderable.h"
#include "engine/net/snapshottable.h"
#include "../../../engine/audio/sound_instance.h"


class KEntity_Weapon_Blast :
	public KEntity_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "blastertex">>
#endif
{
public:
	SNAP_PROP(bool, bFrenzySound = false, SNAP_SEND_OTHERS)
private:
	GFlt Charge = 0;

	KSoundInstance FrenzyLoop;

	bool Fire() override;
	bool AltFire() override;

	void PrimaryFireCosmetics() override;
	void AltFireCosmetics() override;
	
public:

	KEntity_Weapon_Blast();
	~KEntity_Weapon_Blast();

	void Tick() override;

	void Poolable_PreDestroy() override;
	
	void AddCharge(GFlt charge);

	u32 GetMaxAmmo() const override { return 500; }
	u32 GetRegenCount() const override { return 5; }

	bool HasFrenzyPowerup();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "blastertex">& entry) override;
#endif
};