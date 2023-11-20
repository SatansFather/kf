#pragma once

#include "engine/game/ent_creation.h"
#include "../entity.h"

class KMapEntity_ProjectileLauncher : public KLoadedMapEntity
{
	
public:
	
	class KMapEntity_PortalExit* Target = nullptr;
	KString TargetName;
	u32 Interval = 60;
	u32 InitialDelay = 60;

	void LinkTarget() override;
	void CreateLauncher();
};

class KEntity_ProjectileLauncher : public KEntity
{
public:
	KMapEntity_ProjectileLauncher* MapEntity = nullptr;
	u32 Interval = 60;
	u32 InitialDelay = 60;
	void Tick() override;
};