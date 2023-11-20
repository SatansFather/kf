#include "death_cam.h"
#include "engine/game/local_player.h"
#include "engine/input/view.h"

#if !_SERVER

KEntity_DeathCamera::KEntity_DeathCamera()
{
	PhysicsProperties.Deceleration = .86;
	PhysicsFluidProperties.Deceleration = .93;
	SetMovementState(EMoveState::Physics);

	SetCollisionBoundsHalfExtent(14);

	//CollisionChannels |= ECollisionMask::PlayerCharacter;
	CollisionBlocks |= ECollisionMask::WorldStatic;
	//CollisionBlocks |= ECollisionMask::MonsterCharacter;
	CollisionOverlaps |= ECollisionMask::Portal;
	CollisionOverlaps |= ECollisionMask::Launcher;
	CollisionOverlaps |= ECollisionMask::Water;

	GetLocalPlayer()->CameraRoll = 0;
	//GetLocalPlayer()->bTeleportedThisFrame = true;

	KInputView::LockInput();
}

KEntity_DeathCamera::~KEntity_DeathCamera()
{
	GetLocalPlayer()->bPendingControlReset = false;
	KInputView::UnlockInput();
}

void KEntity_DeathCamera::OnMoveBlocked(const GVec3& preVel, const GHitResult& hit)
{
	bHasHit = true;
}

void KEntity_DeathCamera::Tick()
{
	PerformMovement(0);


	//if (IsInWater()) bHasHit = true;
	const GFlt maxRollTime = 2;
	GFlt rollAdjust = (GameFrameDelta() / 3);
	if (GetEntityAge() < maxRollTime)
	{
		GFlt rollDecel = maxRollTime - GetEntityAge();
		rollDecel /= maxRollTime;
		rollAdjust *= rollDecel;
	}
	else
		rollAdjust = 0;

	KInputView::SetPitch(0);

	GetLocalPlayer()->CameraRoll += bHasHit ? 0 : rollAdjust;

	GetLocalPlayer()->CameraPosition = GetPosition().AdjustZ(OffsetZ);
	if (TeleportedThisFrame())
		GetLocalPlayer()->bTeleportedThisFrame = true;

	if (GetEntityAge() > 1)
		GetLocalPlayer()->bPendingControlReset = true;
}

#endif