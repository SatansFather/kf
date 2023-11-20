#include "character_monster.h"
#include "engine/game/local_player.h"

// DELETE
#include "engine/collision/trace.h"
#include "../../engine/utility/random.h"
#include "spectator.h"
#include "../../engine/net/player.h"
#include "../../engine/game/kill_feed_message.h"
#include "../../engine/game/frag_message.h"
#include "../../engine/net/net_interface.h"

KEntity_Character_Monster::KEntity_Character_Monster()
{
	WalkingProperties.MoveSpeed = 200;
	WalkingProperties.Deceleration = .875;
	WalkingProperties.Acceleration = 0;
	//GetWalkingMovement()->WalkingProperties.Acceleration = 3500;
	AirAcceleration = 0;
	SetMovementState(EMoveState::Walking);
	//SetHealth(10);
	SetCollisionBoundsHalfExtent(GVec3(14, 14, 24));
	CollisionChannels = ECollisionMask::MonsterCharacter;

	CollisionBlocks = ECollisionMask::WorldStatic
		| ECollisionMask::MonsterCharacter
		| ECollisionMask::Weapon
		| ECollisionMask::PlayerCharacter;

	CollisionOverlaps = ECollisionMask::Launcher
		//| ECollisionMask::Gib
		| ECollisionMask::Water
		| ECollisionMask::Portal;

	MaxStepHeight = 24;

	FrameTickInterval = 3;

	SetSimpleMovement(true);
}

GVec3 KEntity_Character_Monster::GetAiInputvector()
{
	if (KTime::FramesSince(LastInputUpdateFrame) > 30 && WanderFramesRemaining == 0)	
	{
		CurrentInputVector = GVec3(RandFloat(-1, 1), RandFloat(-1, 1), 0).GetNormalized();

		KEntity* guy = GetLocalPlayer()->ControlledEntity.Get();
		if (!guy) return 0;

		LastInputUpdateFrame = KTime::FrameCount();

		GVec3 targetPos = guy->GetPosition();
		GVec3 toTarget = (targetPos - GetPosition()).SetZ(0).GetNormalized();

		f32 desiredAngle = glm::radians(22.5f);
		f32 actualAngle = atan2(toTarget.x, toTarget.y);

		f32 newAngle = std::round(actualAngle / desiredAngle) * desiredAngle;
		toTarget = GVec3(sin(newAngle), cos(newAngle), 0);

		CurrentInputVector = toTarget;
	}
	else if (WanderFramesRemaining > 0)
		WanderFramesRemaining--;

	return CurrentInputVector;
}

void KEntity_Character_Monster::CharacterTick()
{
	/*GHitResult hit;
	hit.SearchCollision = ECollisionMask::PlayerCharacter;
	TraceLine(GLineSegment(GetPosition(), GetPosition() + Velocity.GetNormalized() * 50), hit);
	if (hit.HitCollision & ECollisionMask::PlayerCharacter)
	{
		KEntity_Character_Player* guy = (KEntity_Character_Player*)(((KEntProp_CollidableBBox*)hit.Object)->GetEntity());
		if (guy->IsValid())
		{
			guy->Push(Velocity);
			guy->TakeDamage(10);
		}
	}*/
}

void KEntity_Character_Monster::OnMoveBlocked(const GVec3& preVel, const GHitResult& hit)
{
	// generate random vector
	CurrentInputVector = GVec3(D_RandRange(-1, 1), D_RandRange(-1, 1), 0);
	CurrentInputVector.Normalize();

	// snap to angle
	f32 desiredAngle = glm::radians(22.5f);
	f32 actualAngle = atan2(CurrentInputVector.x, CurrentInputVector.y);
	f32 newAngle = std::round(actualAngle / desiredAngle) * desiredAngle;
	CurrentInputVector = GVec3(sin(newAngle), cos(newAngle), 0);

	// face away from hit
	if (hit.Normal.Dot(CurrentInputVector) > 0) 
		CurrentInputVector = -CurrentInputVector;

	WanderFramesRemaining = D_RandRange(15, 25);
}

#if !_SERVER
/*
KBufferUpdateResult KEntity_Character_Monster::UpdateBuffers(KBoundingBoxRender& entry)
{
	if (IsPlayerControlled())
		return KBufferUpdateResult(false, false);

	entry.PrevPos = LastFramePosition.ToGLM4();
	entry.CurrentPos = GetPosition().ToGLM4();
	entry.CurrentPos.w = TeleportedThisFrame();
	entry.HalfExtent = glm::vec4(14, 24, 14, 1);

	return KBufferUpdateResult(LastFramePosition != GetPosition(), true);
}*/

KBufferUpdateResult KEntity_Character_Monster::UpdateBuffers(KJohnnyJiantdick& entry)
{
	if (IsViewedEntity()) return false;

	GVec3 pre;
	GVec3 post;

	//if (IsReadingReplay())
	//{
	//	pre = GetPosition();
	//	post = GetPosition();
	//}
	//else
	{
		pre = GVec3::Lerp(LastFrameRenderPosition, GetPosition(), f32(FramesSinceMove) / f32(FrameTickInterval));
		post = GVec3::Lerp(LastFrameRenderPosition, GetPosition(), f32(FramesSinceMove + 1) / f32(FrameTickInterval));
	}

	entry.PrevPos = pre.ToGLM4();
	entry.CurrentPos = post.ToGLM4();

	entry.PrevPos.w = TeleportedThisFrame();
	entry.CurrentPos.w = 1;//GetLastMoveAlpha();

	return KBufferUpdateResult(pre != post, true);
}

#endif