#pragma once

#include "engine/math/vec3.h"
#include "engine/game/ent_creation.h"

class KMapEntity_Launcher : public KLoadedMapEntity
{
	
public:
	
	class KMapEntity_PortalExit* Target = nullptr;
	KString TargetName;
	GFlt LaunchArc = .5;

	GVec3 GetLaunchVelocity(const GVec3& offset = GVec3(0, 0, 0), GFlt gravityScale = 1) const;
	
	void LinkTarget() override;
};