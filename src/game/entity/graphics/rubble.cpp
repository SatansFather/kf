#if !_SERVER
#include "rubble.h"
#include "engine/utility/random.h"

KEntity_Rubble::KEntity_Rubble()
{
	RotationRate = glm::vec3(RandFloat(-1, 1), RandFloat(-1, 1), RandFloat(-1, 1)) * 360.f;
	Rotation = glm::quat(RotationRate);
	GravityScale = 1;
	SetCollisionBoundsHalfExtent(1);
	PhysicsProperties.Deceleration = .8;
	PhysicsFluidProperties.Deceleration = .7;
	SetMovementState(EMoveState::Physics);
	Bounciness = 1;
	//CollisionChannels = ECollisionMask::Rubble;
	CollisionBlocks |= ECollisionMask::WorldStatic;
	CollisionOverlaps |= ECollisionMask::Portal;
	CollisionOverlaps |= ECollisionMask::Water;
	CollisionOverlaps |= ECollisionMask::Launcher;
	RenderableLifespan = RandFloat(2.5, 3.5);
	FrameTickInterval = 4;
}

void KEntity_Rubble::CreateExplosion(const GVec3& position, const GVec3& normal)
{
	for (i32 i = 0; i < 32; i++)
	{
		auto chunk = TDataPool<KEntity_Rubble>::GetPool()->CreateNew().Get();
		chunk->SetPosition(position);
		chunk->LastFrameRenderPosition = position;
		chunk->BuildMatrix(chunk->PrevMatrix);
		GVec3 pushDir = (normal * 3) + GVec3(RandRange(-2, 2), RandRange(-2, 2), RandRange(-2, 2));
		pushDir.z += 1;
		chunk->Push(pushDir * RandRange(100, 150));
	}
}

KBufferUpdateResult KEntity_Rubble::UpdateBuffers(KStaticMesh<"meat1", "kf/testrock5">& entry)
{
	entry.PrevModelMat = PrevMatrix;
	BuildMatrix(entry.CurrentModelMat);

	PrevMatrix = entry.CurrentModelMat;

	entry.SetLastMoveRenderAlpha(1);
	return KBufferUpdateResult(LastFrameRenderPosition != GetPosition(), true);
}

void KEntity_Rubble::BuildMatrix(glm::mat4& matrix)
{
	matrix = glm::mat4(1);

	const GVec3 post = GVec3::Lerp(LastFrameRenderPosition, GetPosition(), f32(FramesSinceMove + 1) / f32(FrameTickInterval));

	matrix = glm::translate(matrix, post.ToGLM());
	matrix = glm::scale(matrix, glm::vec3(2, 2, 2) * KSaturate(RenderableLifespan - GetEntityAge()));

	glm::mat4 rot(1);

	if (IsInWater())
	{
		RotationRate *= .82;
		glm::quat delta = glm::quat(glm::radians(RotationRate) * (f32)(GameFrameDelta()));
		Rotation = delta * Rotation;
	}
	else if (MovementIsEnabled())
	{
		if (PhysicsFloorNormal != 0)
		{
			RotationRate.x = -Velocity.x;
			RotationRate.y = 0.f;
			RotationRate.z = Velocity.y;
			RotationRate *= -2 * PI<f32>();
		}

		glm::quat delta = glm::quat(glm::radians(RotationRate) * (f32)(GameFrameDelta()));
		Rotation = delta * Rotation;
	}

	rot = glm::mat4_cast(Rotation);
	matrix *= rot;
}

void KEntity_Rubble::Tick()
{
	PerformMovement(0);
		
	if ((PhysicsFloorNormal.z > 0 && Velocity.SetZ(0).LengthSq() < .5) ||
		(IsInWater() && Velocity.LengthSq() < 5))
		SetMovementEnabled(false);
}

#endif