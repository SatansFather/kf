#pragma once

#include "../projectile.h"
#include "../graphics/rocket_trail.h"
#include "engine/audio/sound_instance.h"
#include "engine/net/snapshottable.h"

class KEntity_Projectile_Rocket :
	public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"rocket", "kf/colors/color_red">, KDynamicLight>
#endif
{
public:

	SNAP_PROP(KNetVec3, DestroyPosition, SNAP_DESTROY, SNAP_SEND_OTHERS)
	SNAP_PROP(KNetVec3, DestroyNormal, SNAP_DESTROY, SNAP_SEND_OTHERS)

	glm::mat4 PrevMatrix = glm::mat4(1.f);
	f32 SplashRadius = 150;
	i32 MaxSplashDamage = 150;

	KSoundInstance LoopSound;

#if !_SERVER
	TObjRef<KEntity_RocketTrail> Trail;
#endif

public:

	KEntity_Projectile_Rocket();
	~KEntity_Projectile_Rocket();

	void Poolable_OnInstantiated() override;
	void Poolable_PreDestroy() override;
	void InitNetObject();
	static TObjRef<KEntity_Projectile_Rocket> Create(const KProjectileCreationParams& params);

	void OnNetDestroy();

	void BuildMatrix(glm::mat4& matrix);
	void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;

	void PreClientLocalTick(u32 frames) override;

#if !_SERVER
	TObjRef<KEntity_ShotgunTrail> SmokeBeam;
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"rocket", "kf/colors/color_red">& entry, KDynamicLight& light) override;
#endif
};