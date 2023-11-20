#if !_SERVER

#include "gibs.h"
#include "engine/utility/random.h"
#include "blood_trail.h"
#include "water_splash.h"

KEntity_GibBase::KEntity_GibBase()
{
	PhysicsProperties.Deceleration = .96;
	PhysicsFluidProperties.Deceleration = .93;
	SetMovementState(EMoveState::Physics);

	Bounciness = 1.5;
	SetCollisionBoundsHalfExtent(4);
	//FrameTickInterval = 3;
	CollisionChannels = ECollisionMask::Gib;
	CollisionBlocks |= ECollisionMask::WorldStatic;
	//CollisionBlocks |= ECollisionMask::PlayerCharacter;
	CollisionOverlaps |= ECollisionMask::Portal;
	CollisionOverlaps |= ECollisionMask::Water;
	CollisionOverlaps |= ECollisionMask::Launcher;

	RotationRate = glm::vec3(RandFloat(-1, 1), RandFloat(-1, 1), RandFloat(-1, 1)) * 360.f;
	Rotation = glm::quat(RotationRate);
}

void KEntity_GibBase::Tick()
{
	//if (GetEntityAge() > .25) CollisionBlocks |= ECollisionMask::Gib;

	PerformMovement(0);
	if ((PhysicsFloorNormal.z > 0 && Velocity.SetZ(0).LengthSq() < .5) ||
		(IsInWater() && Velocity.LengthSq() < 5) )
		SetMovementEnabled(false);

	if (GetEntityAge() < (IsInWater() ? .5 : 1.5) && PhysicsFloorNormal == 0)
	{
		if (IsInWater() && GetWaterPenetration() > 24)
			KEntity_BloodTrail_UnderWater::Create(GetPosition());
		else
			KEntity_BloodTrail::Create(GetPosition());
	}
}

void KEntity_GibBase::OnEnteredWater(const GHitResult& hit)
{
	return;
	GFlt velLen = Velocity.Length();
	if (velLen > 300)
		KEntity_WaterSplash::Create(hit.Point - hit.Normal, hit.Normal, FColor32(.72, .52, .68, .35).To8(), velLen / 400, velLen / 80);
}

glm::mat4 KEntity_GibBase::BuildMatrix(const GVec3& lastPos)
{
	if (GetPosition() == lastPos) return PrevMatrix;

	glm::mat4 mat = glm::mat4(1);

	f32 pitch = 0;
	f32 yaw = 0;
	f32 roll = 0;

	GVec3 pos = GetPosition();//GVec3::Lerp(lastPos, GetPosition(), f32(FramesSinceMove + 1) / f32(FrameTickInterval));

	mat = glm::translate(mat, (pos).ToGLM());
	mat = glm::scale(mat, glm::vec3(5, 5, 5));
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
	mat *= rot;

	PrevMatrix = mat; // PrevMatrix was already used before this
	return mat;
}


void KEntity_Gib_MeatChunk::ExplodePush(const GVec3& dir, f32 strength)
{
	strength *= RandRange(.8, 1.2);

	SetMovementEnabled(true);
	KEntProp_Movable::ExplodePush(dir, strength);
	if (PendingImpulse.z < strength)
		PendingImpulse.z = strength;
}

#endif