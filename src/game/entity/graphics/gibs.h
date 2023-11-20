#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"
#include "../properties/movable.h"
#include "../properties/move_states.h"
#include "../properties/collidable.h"

class KEntity_GibBase : public KEntity,
	public KEntProp_Movable,
	public KMoveState_Physics,
	public KEntProp_CollidableBBox
{
public: // protected
	glm::mat4 PrevMatrix = glm::mat4(1);
	glm::quat Rotation;
	glm::vec3 RotationRate;
	glm::vec3 VelRight;
	f32 ContactSpeed = 0;

	glm::mat4 BuildMatrix(const GVec3& lastPos);
public:
	KEntity_GibBase();
	void Tick() override;
	void OnEnteredWater(const GHitResult& hit) override;
};

class KEntity_Gib_MeatChunk : public KEntity_GibBase,
	public KEntProp_Renderable<KStaticMesh<"meat1", "meat1">>
{
public:
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"meat1", "meat1">& entry) override
	{
		entry.PrevModelMat = PrevMatrix;
		entry.CurrentModelMat = BuildMatrix(LastFrameRenderPosition);
		entry.SetLastMoveRenderAlpha(1);
		return true;
	}

	 void ExplodePush(const GVec3& dir, f32 strength) override;
};

#endif