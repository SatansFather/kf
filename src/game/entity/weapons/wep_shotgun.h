#pragma once

#include "../weapon.h"
#include "../properties/renderable.h"
#include "engine/net/snapshottable.h"

class KEntity_Weapon_Shotgun :
	public KEntity_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "shotguntex">>
#endif
{	

	bool Fire() override;
	bool AltFire() override;

	void PrimaryFireCosmetics() override;
	void AltFireCosmetics() override;
	
public:

	KEntity_Weapon_Shotgun();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "shotguntex">& entry) override;
#endif
};