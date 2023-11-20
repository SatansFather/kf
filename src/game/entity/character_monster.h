#pragma once

#include "character.h"
#include "properties/renderable.h"
#include "engine/net/snapshottable.h"

class KEntity_Character_Monster :
	public KEntity_Character,
	public KMoveState_Walking
#if !_SERVER
	, public KEntProp_Renderable<KJohnnyJiantdick>
#endif
{
	TObjRef<KEntity> Target;
	GVec3 CurrentInputVector;
	u32 LastInputUpdateFrame = 0;
	u32 WanderFramesRemaining = 0;

public:

	KEntity_Character_Monster();

	GVec3 GetAiInputvector() override;
	void CharacterTick() override;
	void OnMoveBlocked(const GVec3& preVel, const GHitResult& hit) override;
#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KJohnnyJiantdick& entry) override;
#endif
};