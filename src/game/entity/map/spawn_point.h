#pragma once

#include "engine/game/ent_creation.h"

class KMapEntity_DeathmatchSpawn : public KLoadedMapEntity
{
public:
	KMapEntity_DeathmatchSpawn();

	GFlt Closest = -1;
};