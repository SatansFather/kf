#pragma once

#if !_SERVER

#include "../projectile.h"
#include "engine/net/snapshottable.h"
#include "engine/math/packed_float.h"

class KEntity_Rubble : public KEntity,
	public KEntProp_Movable,
	public KMoveState_Physics,
	public KEntProp_CollidableBBox,
	public KEntProp_Renderable<KStaticMesh<"meat1", "kf/testrock5">>
{
	glm::mat4 PrevMatrix = glm::mat4(1.f);
	glm::quat Rotation;
	glm::vec3 RotationRate;

public:

	KEntity_Rubble();
	static void CreateExplosion(const GVec3& position, const GVec3& normal);
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"meat1", "kf/testrock5">& entry) override;
	void BuildMatrix(glm::mat4& matrix);
	void Tick() override;
};

#endif