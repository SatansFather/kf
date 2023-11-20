#pragma once

#include "../projectile.h"
#include "engine/net/snapshottable.h"
#include "engine/math/packed_float.h"

class KEntity_Projectile_ShotgunBoulder :
	public KEntity_Projectile
#if !_SERVER
	, public KEntProp_Renderable<KStaticMesh<"meat1", "kf/stone2">>
#endif
{
	glm::mat4 PrevMatrix = glm::mat4(1.f);
	glm::quat Rotation;
	glm::vec3 RotationRate;

public:

	KEntity_Projectile_ShotgunBoulder();

	void ProjectileTick() override;
	void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) override;
	void InitNetObject();
	static TObjRef<KEntity_Projectile_ShotgunBoulder> Create(const KProjectileCreationParams& params);

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"meat1", "kf/stone2">& entry) override;
#endif

	void BuildMatrix(glm::mat4& matrix);

	void Poolable_PreDestroy() override;
};