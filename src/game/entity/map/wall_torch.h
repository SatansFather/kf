#pragma once

#if !_SERVER

#include "engine/game/ent_creation.h"

class KMapEntity_WallTorch : public KLoadedMapEntity
{
public:
	
	KString MainColor = "10 10 0 5", OffColor = "1 0 0 100";
	void CreateTorch();
};

#endif