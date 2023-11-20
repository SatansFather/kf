#pragma once

#include "engine/game/ent_creation.h"
#include "engine/audio/audio.h"

class KMapEntity_PortalExit : public KLoadedMapEntity
{
public:
	
};

class KMapEntity_Portal : public KLoadedMapEntity
{
public:

	KSoundInstance AmbientInstance;
	KMapEntity_PortalExit* Target = nullptr;
	KString TargetName;
	bool bResetVelocity = false;
	bool bSilent = false;
	f32 TimeScale = 4;
	f32 RippleRate = 1.5;
	f32 HeightRatio = 1;

	void LinkTarget() override;
	void OnCollisionBrushAssociation() override;
	void UpdateSoundPosition(const GVec3& pos) override;
};