#pragma once

#include "../projectile.h"
#include "engine/audio/sound_instance.h"
#include "engine/net/snapshottable.h"

class KEntity_Projectile_Cannon : 
	public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"cannonball", "kf/stone1">/*, KDynamicLight*/>
#endif
{
public:

	glm::mat4 PrevMatrix = glm::mat4(1.f);
	f32 SplashRadius = 150;
	f32 PushScale = 400;
	u32 DestroyFrame = MAX_U32;
	glm::quat Rotation;
	glm::vec3 RotationRate;
	bool bExploded = false;

public:

	KEntity_Projectile_Cannon();
	~KEntity_Projectile_Cannon();

	void Poolable_OnInstantiated() override;
	void Poolable_PreDestroy() override;
	void InitNetObject();
	static TObjRef<KEntity_Projectile_Cannon> Create(const KProjectileCreationParams& params);

	void BuildMatrix(glm::mat4& matrix);
	void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;

	bool IsMyProjectile() const { return HasObjectAuthority(); }

#if !_SERVER
	TObjRef<KEntity_ShotgunTrail> SmokeBeam;
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"cannonball", "kf/stone1">& entry/*, KDynamicLight& light*/) override;
#endif

	void OverlapCharacter(const GVec3& preVel, GHitResult& hit) override;
};

