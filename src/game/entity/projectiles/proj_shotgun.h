#pragma once

#include "../projectile.h"
#include "engine/net/snapshottable.h"
#include "engine/math/packed_float.h"

class KEntity_Projectile_ShotgunShard : 
	public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KShotgunShard>
#endif
{

	u32 LastBounceFrame = 0;
	u8 BouncesRemaining = 5;

public:

	void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;
	void InitNetObject();
	static TObjRef<KEntity_Projectile_ShotgunShard> Create(const KProjectileCreationParams& params);

#if !_SERVER
	TObjRef<KEntity_ShotgunTrail> SmokeBeam;
	KBufferUpdateResult UpdateBuffers(KShotgunShard& entry) override;
#endif

	void OnEntityDestroyed() override;
};