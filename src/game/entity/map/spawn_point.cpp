#include "spawn_point.h"
#include "engine/game/match.h"

KMapEntity_DeathmatchSpawn::KMapEntity_DeathmatchSpawn()
{
	GetGameMatch()->SpawnPoints.push_back(this);
}
