#pragma once

#include "engine/game/ent_creation.h"

class KMapEntity_PickupItem : public KLoadedMapEntity
{
public:
	
	void CreatePickup(const KString& name);
	
};
