#pragma once

#include "../weapon.h"
#include "../properties/renderable.h"

class KEntity_Weapon_Zapper : public KEntity_Weapon
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"gunplaceholder", "zappertex">>
#endif
{	
	bool Fire() override;
	bool AltFire() override;

	void PrimaryFireCosmetics() override;
	void AltFireCosmetics() override;
	
public:

	KEntity_Weapon_Zapper();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"gunplaceholder", "zappertex">& entry) override;
#endif
};