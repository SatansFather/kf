#pragma once

#include "../weapon.h"
#include "../properties/renderable.h"
#include "engine/net/snapshottable.h"
#include "../projectiles/proj_cannon.h"

class KEntity_Weapon_Cannon :
	public KEntity_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "cannontex">>
#endif
{	
	friend class KEntity_Projectile_Cannon;
	TVector<TObjRef<KEntity_Projectile_Cannon>> Projectiles;

	bool Fire() override;
	bool AltFire() override;

	void PrimaryFireCosmetics() override;
	void AltFireCosmetics() override;

	bool RequireAmmoForFire(bool primary) const override { return primary; }

public:

	KEntity_Weapon_Cannon();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "cannontex">& entry) override;
#endif
};