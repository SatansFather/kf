#if 1

#pragma once

#include "engine/audio/sound_instance.h"
#include "../projectile.h"
#include "../properties/renderable.h"

class KEntity_Projectile_Blast :
	public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KBlasterParticle, KDynamicLight>
#endif
{
public:

	f32 RenderTimeCreated = 0;

	SNAP_PROP(KNetVec3, DestroyPosition, SNAP_DESTROY, SNAP_SEND_OTHERS)

public:

	KEntity_Projectile_Blast();
	~KEntity_Projectile_Blast();

	void InitNetObject();
	static TObjRef<KEntity_Projectile_Blast> Create(const KProjectileCreationParams& params);

	//void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;

	bool IsMyProjectile() const { return HasObjectAuthority(); }

	void OnNetDestroy() override;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KBlasterParticle& entry, KDynamicLight& light) override;
#endif

};

#endif

