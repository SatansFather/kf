#pragma once
#include "../projectile.h"

class KEntity_Projectile_Atom : public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KAtomProjectile, KDynamicLight>
#endif
{
	f32 StopTime = -1;

public:

	KEntity_Projectile_Atom();
	
	void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;

	bool IsMyProjectile() const { return true; }

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KAtomProjectile& entry, KDynamicLight& light) override;
#endif
};