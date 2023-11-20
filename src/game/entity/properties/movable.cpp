#include "movable.h"
#include "collidable.h"
#include "../entity.h"
#include "engine/collision/trace.h"
#include "engine/input/listener_game.h"
#include "engine/input/binding.h"
#include "engine/game/local_player.h"
#include "engine/utility/function_arg.h"


// DELETE
#include "engine/system/terminal/terminal.h"
#include "engine/game/match.h"
#include "../map/portal.h"
#include "controllable.h"
#include "../../../engine/net/net_interface.h"
#include "../graphics/gibs.h"
#include "../../../engine/utility/random.h"
#include "../character.h"
#include "../pickup.h"
#include "../map/launcher.h"
#include "../../cheat_manager.h"
#include "engine/audio/audio.h"
#include "engine/game_instance.h"
#include "../graphics/portal_travel.h"

#if !_SERVER
#include "engine/input/view.h"
#endif
#include "../../../engine/net/player.h"
#include "../projectile.h"
#include "../../../engine/collision/broadphase/trace_args.h"

// spot on boxtest at edge of red rocks - running against wall is really really slow
// entity touch should trigger on successful step move
// register overlaps if first touch is solid

#define TINY_STEP_MOVE .5

// prevent dynamic casting every move
// this will break if movement is done in multiple threads, but it probably wont be
KEntity* ThisMoveEntity = nullptr;
KEntProp_CollidableBBox* ThisMoveCollision = nullptr;
KMoveState_Flying* ThisMoveFlying = nullptr;
KMoveState_Walking* ThisMoveWalking = nullptr;
KMoveState_Swimming* ThisMoveSwimming = nullptr;
KMoveState_Physics* ThisMovePhysics = nullptr;

KEntProp_Movable::KEntProp_Movable()
{
	ThisFrameHitEntities.reserve(4);
	SetMovementEnabled(true);
	Movable_GroundCheck();
}

void KEntProp_Movable::Movable_SlideOnLanding(const GVec3& norm)
{
	if (IsFalling())
	{
		// view shake
		if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(this))
		{
			if (cont->IsViewedEntity())
			{
				f32 landZ = KClamp(abs(Velocity.z / 100), 0, 16);
				if (landZ >= LandZ)
				{
					LastLandFrame = KTime::FrameCount();
					LandZ = KClamp(abs(Velocity.z / 100), 0, 16);

					GVec3 lookDir = GVec3::FromPitchYaw(0, cont->GetYaw());
					GVec3 rightDir = lookDir.Cross(GVec3(0, 0, 1)).GetNormalized();
					f32 roll = (Velocity / 100).Dot(rightDir);
					roll = KClamp(roll, -3, 3);
					GetLocalPlayer()->PushCameraRotation(FVec3(-2, 0, roll) * pow(LandZ / 16, 2), .3 * LandZ / 16, 0);
				}
			}
		}

		// slide
		if (norm.z < 1) ClipVector(Velocity, norm, Velocity);
	}
}

bool KEntProp_Movable::Movable_MoveTrace(const GVec3& start, const GVec3& finish, GHitResult& hit, u8& resolves, KTraceCondition condition)
{
	KEntProp_CollidableBBox* col = Movable_GetCollision();
	GFlt crouchDist = col->CrouchDistance;
	bool noOverlap = (MovableFlags & MF_GroundTrace) || (MovableFlags & MF_TestingStepUp);
	col->InitHitResult(hit, true, !noOverlap);	

	bool inWater = false;
	GHitResult waterHit;
	const auto testHitResult = [](KTraceHitParams& hits, KMoveTraceLocalCapture& locals) -> bool
	{
		if (locals.Collidable && hits.Test->Object == locals.Collidable) return false;
		if (hits.Test->HitCollision == ECollisionMask::Gib && *locals.Resoves < 6) return false; // dont let gibs ruin the move
		//if (hits.Test->StartsSolid() && locals.Collidable->BlocksChannel(hits.Test->HitCollision)) return false;

		{
			if ((locals.Collidable->GetCollisionOverlaps() & hits.Test->HitCollision))
			{
				// check swimming
				if (hits.Test->HitCollision & ECollisionMask::Water)
				{
					if (hits.Test->PenetrationDepth >= 0 && locals.Movable->LastWaterPenetration < 0)
						*locals.WaterHit = *hits.Test;

					locals.Movable->WaterPenetration = hits.Test->PenetrationDepth;// + hits.Test->PositionOffsetZ;
					locals.Movable->LastWaterPenetration = locals.Movable->WaterPenetration;

					GFlt extentZ = locals.Collidable->GetDefaultHalfExtent().z;

					if (locals.Movable->WaterPenetration >= extentZ)
						*locals.bInWater = true;
				}
				else if (hits.Test->HitCollision == ECollisionMask::Pickup && locals.Collidable->OverlapsChannel(ECollisionMask::Pickup))
				{
					if (hits.Test->StartsSolid())
					{
						// if we're inside an item, try to pick it up now instead of colliding with it
						locals.Movable->ThisFrameHitEntities.push_back(hits.Test->Object);
						KEntity_PickupBase* pickup = dynamic_cast<KEntity_PickupBase*>(((KEntProp_CollidableBBox*)hits.Test->Object)->GetEntity());
						KEntity* ent = locals.Movable->GetEntity();
						locals.PendingItems->push_back(pickup);
						return false;
					}
				}

				if (hits.Test->StartsSolid()) return false;
			}
		}

		bool cond = locals.Condition ? locals.Condition(*hits.Prev, *hits.Test) : true;

		if (cond && hits.Test->Object)
		{
			if (VectorContains(locals.Movable->ThisFrameHitEntities, hits.Test->Object))
				return false;
			
			return true;
		}
		return false;
	};

	KMoveTraceLocalCapture cap;
	cap.Movable = this;
	cap.Collidable = col;
	cap.bInWater = &inWater;
	cap.WaterHit = &waterHit;
	cap.Resoves = &resolves;
	cap.Condition = condition;

	TVector<TObjRef<KEntity_PickupBase>> pendingPickups;
	cap.PendingItems = &pendingPickups;

	KFunctionArg<KTraceHitParams, KMoveTraceLocalCapture, bool> func(testHitResult, cap);

	TraceBox(GLineSegment(start, finish), col->GetCollisionBoundsHalfExtent(), hit, &func, CompFrames);

	for (TObjRef<KEntity_PickupBase>& pickup : pendingPickups)
	  if (KEntity_PickupBase* p = pickup.Get())
	    p->OnOverlap(GetEntity());

	if (!noOverlap)
	{
		if (waterHit.bHit) OnEnteredWater(waterHit);

		if (inWater) 
			MovableFlags |= MF_InsideFluid;
		else		 
			MovableFlags &= ~MF_InsideFluid;
	}

	if (IsPhysics())
	{
		Movable_UpdatePhysicsMovementProperties();
	}
	else if (inWater)
	{
		SetMovementState(EMoveState::Swimming);
	}
	else if (MovementState == EMoveState::Swimming)
	{
		SetMovementState(EMoveState::Falling);
		if (KEntProp_Controllable* control = dynamic_cast<KEntProp_Controllable*>(this))
		{
			if (control->GetUpInput() > 0)
			{
				// push up a bit if vel z cant take us out of water
				GFlt distToClimb = col->GetCollisionBoundsHalfExtent().z * 2;
				GFlt neededZ = sqrt(2 * GetGravityZ() * distToClimb) / GravityScale;

				if (Velocity.z < neededZ)
					Velocity.z = neededZ;
			}
		}
	}

	if (col->OverlapsChannel(hit.HitCollision))
		return OnOverlapChannel(hit.HitCollision, Velocity, hit);

	return true;
}

void KEntProp_Movable::Movable_UpdatePhysicsMovementProperties()
{
	if (auto movement = GetPhysicsMovement())
	{
		MovementState = EMoveState::Physics;
		MovementProperties = IsInWater() ? movement->PhysicsFluidProperties : movement->PhysicsProperties;
		MovementProperties.ScaleWithSurface(GetMovementSurfaceMods());
	}
}

GFlt KEntProp_Movable::Movable_GetMaxStepHeight()
{
	if (IsFalling())
	{
		KEntity* ent = GetEntity();
		KEntProp_CollidableBBox* col = ent->As<KEntProp_CollidableBBox>();

		
		GHitResult hit;
		col->InitHitResult(hit);

		const GFlt graceHeight = MaxStepHeight / 3;
		GFlt maxStepZ = 0;

		// cannot step higher than MaxStepHeight above the ground underneath us
		//TraceBox(GLineSegment(ent->GetPosition().AdjustZ(-col->CrouchDistance/2), ent->GetPosition().AdjustZ(-col->CrouchDistance/2).AdjustZ(-MaxStepHeight)),
		//	col->GetCollisionBoundsHalfExtent(), hit);
		//
		//if (hit.bHit) hit.Point.z += col->CrouchDistance/2;

		TraceBox(GLineSegment(ent->GetPosition(), ent->GetPosition().AdjustZ(-MaxStepHeight)),
			col->GetCollisionBoundsHalfExtent(), hit);

		if (hit.bHit && IsWalkable(hit.Normal))
		{
			GFlt maxStepPos = hit.Point.z + MaxStepHeight;
			maxStepZ = maxStepPos - ent->GetPosition().z;
		}

		if (Velocity.z > 0)
		{
			// if ascending, max step height is clamped by our remaining distance to ascend to apex
			const GFlt distFromApex = pow(Velocity.z, 2) / (2 * GetGravityZ());
			const GFlt height = graceHeight + distFromApex;
			maxStepZ = KMax(maxStepZ, KMin(MaxStepHeight, height));
		}

		return KMax(maxStepZ, graceHeight);
	}

	return MaxStepHeight;
}

class KEntProp_CollidableBBox* KEntProp_Movable::Movable_GetCollision()
{
	return GetProperty<KEntProp_CollidableBBox>();
}

bool KEntProp_Movable::Movable_Bounce(const GVec3& norm)
{
	if (IsPhysics())
	{
		GFlt into = Velocity.Dot(-norm) * .25;
		if (into < 0) return false;

		GVec3 add = into * norm * GetPhysicsMovement()->Bounciness;
		if (add.LengthSq() < 2) return false;
		
		ClipVector(Velocity, norm, Velocity);
		Velocity += add;

		/*GVec3 vel = Velocity;
		vel = vel.Reflect(norm);

		flt_t into = vel.GetNormalized().Dot(norm);
		if (into > 0) return false; // wtf

		vel *= 1 - into;

		Velocity = vel;*/
		return true;
	}

	return false;
}

KMoveState_Walking* KEntProp_Movable::GetWalkingMovement()
{
	return dynamic_cast<KMoveState_Walking*>(this);
}

KMoveState_Swimming* KEntProp_Movable::GetSwimmingMovement()
{
	return dynamic_cast<KMoveState_Swimming*>(this);
}

KMoveState_Flying* KEntProp_Movable::GetFlyingMovement()
{
	return dynamic_cast<KMoveState_Flying*>(this);
}

KMoveState_Physics* KEntProp_Movable::GetPhysicsMovement()
{
	return dynamic_cast<KMoveState_Physics*>(this);
}

KMovementSurfaceModifier KEntProp_Movable::GetMovementSurfaceMods() const
{
	return KMovementSurfaceModifier();
}

void KEntProp_Movable::SetReplayingMoves(bool replaying)
{
	if (replaying)	MovableFlags |= MF_ReplayingMoves;
	else			MovableFlags &= ~MF_ReplayingMoves;
}

void KEntProp_Movable::SetMovementState(EMoveState state)
{
	switch (state)
	{
		case EMoveState::Walking:
		{
			if (auto movement = GetWalkingMovement())
			{
				MovementState = state;
				MovementProperties = movement->WalkingProperties;
				MovementProperties.ScaleWithSurface(GetMovementSurfaceMods());
			}
			break;
		}
		case EMoveState::Flying:
		{
			if (auto movement = GetFlyingMovement())
			{
				MovementState = state;
				MovementProperties = movement->FlyingProperties;
				MovementProperties.ScaleWithSurface(GetMovementSurfaceMods());
			}
			break;
		}
		case EMoveState::Swimming:
		{
			if (auto movement = GetSwimmingMovement())
			{
				MovementState = state;
				MovementProperties = movement->SwimmingProperties;
				MovementProperties.ScaleWithSurface(GetMovementSurfaceMods());
			}
			break;
		}
		case EMoveState::Falling:
		{
			if (auto movement = GetWalkingMovement())
			{
				MovementState = state;
				MovementProperties = movement->WalkingProperties;
				MovementProperties.Acceleration = movement->AirAcceleration;
				MovementProperties.ScaleWithSurface(GetMovementSurfaceMods());
				MovementProperties.Deceleration = 1;
			}
			break;
		}
		case EMoveState::Physics:
		{
			Movable_UpdatePhysicsMovementProperties();
		}
	}
}

void KEntProp_Movable::ClipVector(const GVec3& in, const GVec3& norm, GVec3& out)
{
	GFlt backoff = in | norm;
	if (backoff < 0) backoff *= 1.001;
	else backoff /= 1.001;
	out = in - (norm * backoff);

	//if (IsPhysics()) out = in.Reflect(norm).GetNormalized() * out.Length();
}

GVec3 KEntProp_Movable::RedirectAlongFloor(const GVec3& trace)
{
	if (IsWalking())
	{
		if (auto s = GetWalkingMovement())
		{
			if (s->WalkingFloorNormal.z != 0 && s->WalkingFloorNormal.z < 1)
			{
				GVec3 a = trace ^ GVec3(0, 0, 1);
				GVec3 b = s->WalkingFloorNormal ^ a;
				b.Normalize();
				b *= trace.Length() / b.SetZ(0).Length();

				if (b == b) // NaN check
				{
					GVec3 traceEnd = b;

					// small epsilon prevents tracing into the floor
					// drifting away is prevented by ground checks
					traceEnd.z += .001;

					return traceEnd;
				}
			}
		}
	}
	return trace;
}	

bool KEntProp_Movable::Movable_GroundCheck()
{
	if ((!IsGroundMovement() && !IsPhysics()) || ShouldSkipGroundTrace()) return true;
	KEntProp_CollidableBBox* col = Movable_GetCollision();
	KEntity* ent = GetEntity();
	if (!col || !col->IsCollisionEnabled() || !ent) return true;

	GVec3 start = ent->GetPosition();

	// if our ground is slightly slanted and were against a wall that is slightly slanted away
	//		the wall may be registered as the ground plane despite having a valid floor
	const auto walkableTest = [](const GHitResult& prev, const GHitResult& test) -> bool
	{
		if (test.HitCollision & ECollisionMask::Launcher) return false;

		if (prev) // favor the highest norm z
		{
			if (prev)
			  if (abs(prev.Point.z - test.Point.z) < .01)
			    return test.Normal.z > prev.Normal.z;
			/*bool prevWalkable = prev.Normal.z >= .707;
			bool testWalkable = test.Normal.z >= .707;

			if (testWalkable && prevWalkable)
				return true;
			else if (testWalkable && !prevWalkable)
				return true;
			else if (!testWalkable && !prevWalkable)
				return true;
			else
				return false;*/
		}
		
		return true;
	};

	MovableFlags |= MF_GroundTrace;

	GHitResult hit;
	col->InitHitResult(hit);
	u8 res = 0;

	GFlt down = -.25 * FrameTickInterval;

	TObjRef<KEntity> entRef = ent;
	Movable_MoveTrace(start, start.AdjustZ(IsWalking() ? -(MaxStepHeight + 1) : down), hit, res, walkableTest);
	if (!entRef.IsValid() || !MovementIsEnabled())
		return false;

	MovableFlags &= ~MF_GroundTrace;

	if (hit.IsAllSolid())
	{
		Velocity.z = 0;
		GVec3 point;
		bool corrected = false;
		for (i32 i = -1; i <= 1; i++) 
		{
			for (i32 j = -1; j <= 1; j++) 
			{
				for (i32 k = -1; k <= 1; k++) 
				{
					point = ent->GetPosition();
					point[0] += i;
					point[1] += j;
					point[2] += k;

					// try a box at this position
					GHitResult solidHit;
					col->InitHitResult(solidHit, true, false);
					TraceBox(GLineSegment(point, point), col->GetCollisionBoundsHalfExtent(), solidHit);
					if (!solidHit.IsAllSolid()) 
					{
						Movable_MoveTrace(point, point.AdjustZ(IsWalking() ? -(MaxStepHeight + 1) : -1), hit, res, walkableTest);
						if (!entRef.IsValid() || !MovementIsEnabled())
							return false;

						goto solidCorrected;
					}
				}
			}
		}
		return true;	
	}

solidCorrected:

	// dont continue if we ended up in a different movement mode
	if (!IsGroundMovement() && !IsPhysics()) return true;

	KMoveState_Walking* walkState = GetWalkingMovement();
	KMoveState_Physics* physState = GetPhysicsMovement();

	GVec3& floorNorm = walkState ? walkState->WalkingFloorNormal : physState->PhysicsFloorNormal;
	if (hit.bHit)
	{
		floorNorm = hit.Normal;
		if (IsWalking() && !IsReplayingMoves()) 
			LastMoveStepDistance += hit.Point.z - start.z;
	}
	else
	{
		floorNorm = 0;
	}

	if (IsPhysics()) return true;

	bool walking = hit.bHit && IsWalkable(hit.Normal);

	if (!walking)
	{
		SetMovementState(EMoveState::Falling);
		return true;
	}

	walkState->UpdateFloorNormal(hit.Normal);
	Movable_SlideOnLanding(hit.Normal);

	SetMovementState(EMoveState::Walking);
	ent->SetPosition(ent->GetPosition().SetZ(hit.Point.z));
	return true;
}

GVec3 KEntProp_Movable::Movable_ProcessInput(GVec3 input)
{
	GFlt delta = GameFrameDelta() * FrameTickInterval;

	if (KCheatManager::CheatIsActive(EPersistentCheat::Fly) || KCheatManager::CheatIsActive(EPersistentCheat::Ghost))
		SetMovementState(EMoveState::Flying);
	else if (IsFlying())	// TODO check if cheat was just cleared instead
		SetMovementState(EMoveState::Falling);

	//if (GetGameInput()->GetState() & EInputKeyState::AltFiring)
	//	SetMovementState(EMoveState::Flying);
	//else if (IsFlying())
	//	SetMovementState(EMoveState::Falling);

	if (IsInWater() && IsPhysics())
	{
		auto movement = GetPhysicsMovement();
		GFlt sink = movement->SinkSpeed;

		if (sink > 0 && abs(WaterPenetration) < 50)
		{
			GFlt scale = abs(WaterPenetration) / sink;
			if (scale < .05) scale = 0;
			sink *= scale;
		}
		
		Velocity.z += sink * delta;
	}
	else if (IsFalling() || IsPhysics())
	{
		Velocity.z -= GetGravityZ() * delta;
	}

	if (IsWalking())
	{
		Velocity.z = 0;
		Movable_UpdateCrouchState(input.z < 0);
		if (input.z > 0 && !IsCrouching()) Movable_DoJump();
	}
	else
	{
		MovableFlags &= ~MF_IsCrouching;
	}

	if (IsCrouching() && CrouchDepth < CROUCH_FRAMES)
	{
		CrouchDepth++;
	}
	else if (CrouchDepth > 0 && !IsCrouching())
	{
		if (Movable_CanExitCrouch())
		{
			CrouchDepth--;
		}
		else
		{
			// we couldnt stand up any higher, force crouch
			MovableFlags |= MF_IsCrouching;
			if (CrouchDepth < CROUCH_FRAMES) CrouchDepth++;
		}
	}

	if (IsGroundMovement()) input.z = 0;

	return input.GetNormalized();
}

void KEntProp_Movable::Movable_UpdateVelocity(const GVec3& input)
{
	GFlt delta = GameFrameDelta() * FrameTickInterval;

	bool push = PushFramesRemaining > 0;
	if (push) PushFramesRemaining--;
		
	if (PendingImpulse != 0)
	{
		if (IsWalking() && PendingImpulse.z > 0)
			SetMovementState(EMoveState::Falling);

		Velocity += PendingImpulse;
		PendingImpulse = 0;
	}

	if (IsPhysics())
	{
		Movable_PhysicsMove(input);
		return;
	}


	GFlt maxSpeed = MovementProperties.MoveSpeed;
	GVec3 maxvel = input * maxSpeed;
	GVec3 accel = input * MovementProperties.Acceleration * delta;
	GFlt decel = MovementProperties.Deceleration;

	if (IsCrouching())
	{
		//maxSpeed *= .5;
		//decel -= .085;
		accel *= .5;
	}

	if (push)
	{
		accel *= PushAccelScale;
		decel += (1 - decel) * (1 - PushDecelScale);
		decel = 1;
	}

	// if we're falling on a non walkable plane, dont allow acceleration to let us climb it
	if (IsFalling())
	{
		GVec3 norm = GetWalkingMovement()->WalkingFloorNormal;
		if (norm != 0)
		{
			norm = norm.SetZ(0).GetNormalized();
			if ((norm | accel) < 0)
				ClipVector(accel, norm, accel);
		}
	}

	GFlt z = Velocity.z;
	accel += IsGroundMovement() ? Velocity.SetZ(0) : Velocity;

	//flt_t alen = accel.Normalize();
	//flt_t maxDecel = maxSpeed - (maxSpeed * decel);
	//flt_t deceleration = alen - (alen * decel);
	
	accel *= decel;
	//accel *= alen - KMin(deceleration, maxDecel);

	/*decel = 1 - MovementProperties.Deceleration;
	decel *= 1 - (input | (Velocity.SetZ(0).GetNormalized()));
	accel *= 1 - decel;*/
	
	GFlt templen = accel.Length();
	GFlt m = KMax(maxSpeed + 30, Velocity.SetZ(0).Length());
	if (m <= maxSpeed + 30) m = maxSpeed;
	if (templen > m)
	{
		accel = accel.GetNormalized() * m;
		//accel.z = z;
	}


	Velocity = accel;
	

		/*Velocity.x = KLerp(Velocity.x, maxvel.x, GameFrameDelta() * 10);
		Velocity.y = KLerp(Velocity.y, maxvel.y, GameFrameDelta() * 10);
		Velocity.z = KLerp(Velocity.z, maxvel.z, GameFrameDelta() * 10);
		CHATLOG(Velocity.Length());*/

	// TEMPORARY
	if (MovementProperties.Acceleration == 0) Velocity = maxvel;

	if (IsFalling())
	{
		Velocity.z = z;
	}
	else
	{
		GVec3 vel = Velocity;
		if (IsWalking()) vel.z = 0;
		if (vel.LengthSq() < 25)
			Velocity = 0;
	}
}

void KEntProp_Movable::Movable_PhysicsMove(const GVec3& input)
{
	// less friction on slopes
	GFlt decel = MovementProperties.Deceleration;
	GFlt d = 1 - decel;
	if (!IsInWater())
	{
		const GVec3& norm = GetPhysicsMovement()->PhysicsFloorNormal;
		d *= norm.z;
	}
	decel = 1 - d;
	// accelerate infinitely
	Velocity += input * MovementProperties.Acceleration * (GameFrameDelta() * FrameTickInterval);
	
	Velocity *= GVec3(decel, decel, IsInWater() ? decel : 1);

	/*if (false && norm != 0)
	{
		GVec3 vel = Velocity;
		flt_t len = vel.Normalize();
		len -= GameFrameDelta() * 1000;
		if (len < 0) len = 0;
		Velocity = vel * len;
	}*/
}

GFlt KEntProp_Movable::Movable_MoveAgainstCollision(GFlt remainingMove /*= 0*/)
{
	KEntProp_CollidableBBox* col = Movable_GetCollision();
	KEntity* ent = GetEntity();
	TObjRef<KEntity> entRef = ent;
	if (!col || !col->IsCollisionEnabled()) // no collision, just apply move and return
	{
		ent->SetPosition(ent->GetPosition() + (Velocity * (GameFrameDelta() * FrameTickInterval)));
		return remainingMove;
	}

	bool stepMove = MovableFlags & MF_TestingStepUp;
	bool tinyStep = MovableFlags & MF_TinyStep;

	// keep remaining fraction of move so it can be scaled with delta time
	// remaining is a pointer so we can work with the original value when recursing for steps
	GFlt remaining = remainingMove > 0 ? remainingMove : 1;
	u8 resolves = IsUsingSimpleMovement() ? 2 : 8;

	MoveHitNormals moveHits;
	while (remaining > .0001 && resolves > 0)
	{
		GVec3 startPos = ent->GetPosition();
		GVec3 startVel = Velocity;

		GFlt delta = remaining * (GameFrameDelta() * FrameTickInterval);

		GVec3 traceStart = ent->GetPosition();
		GVec3 traceEnd = traceStart + (Velocity * delta);

		traceEnd = traceStart + RedirectAlongFloor(traceEnd - traceStart);

		GFlt remainingScale = 1;
		if (stepMove)
		{
			if (IsPhysics()) traceEnd.z = traceStart.z;

			// step move is 2 units max, keep it less if move was going to be less
			GFlt stepDist = 2;
			GFlt traceDist;

			// temporarily convert to trace delta rather than destination
			traceEnd = traceEnd - traceStart;

			if (tinyStep)
			{
				traceDist = TINY_STEP_MOVE;
				stepDist = TINY_STEP_MOVE;
			}
			else
			{
				traceDist = traceEnd.Normalize();
				stepDist = KMin(stepDist, traceDist);
			}

			// convert back to destination
			traceEnd *= stepDist;
			traceEnd += traceStart;

			// scale adjustments to remaining since it will be returned to the caller (this function)
			if (traceDist > stepDist) remainingScale = stepDist / traceDist;
		}
		GHitResult hit;
		bool traced = Movable_MoveTrace(traceStart, traceEnd, hit, resolves);

		// check if that trace killed us
		if (!entRef.IsValid() || !MovementIsEnabled())
			return -1;

		remaining -= (hit.Time * remaining) * remainingScale;
		resolves--;

		if (!stepMove) LastMoveAlpha = 1 - remaining;
		if (!traced) continue;

		ent->SetPosition(hit.Point);

		// no blocking hit
		if (hit.Time == 1) break;
		// track hit normals during this move
		if (!moveHits.Update(hit.Normal, this)) continue;

		// block on contact means this will be called even if we can step up
		if (ShouldBlockOnContact() && col->BlocksChannel(hit.HitCollision))
		{
			OnMoveBlocked(Velocity, hit);
		
			if (!entRef.IsValid() || !MovementIsEnabled())
				return -1;
		}

		// if walking or falling, see if we can base on this hit
		if (IsWalkable(hit.Normal) && IsGroundMovement())
		{
			if (auto s = GetWalkingMovement())
			{
				Movable_SlideOnLanding(hit.Normal);

				Velocity.z = 0;
				SetMovementState(EMoveState::Walking);
				s->UpdateFloorNormal(hit.Normal);
				continue;
			}
		}

		// couldnt walk onto hit, try stepping up
		if (!stepMove && (!IsUsingSimpleMovement() || hit.HitCollision & ECollisionMask::WorldStatic))
		{
			i32 stepped = 0;

			// check norm z for consistency, prevents sloped edges from being steppable
			// from quake 1 to unreal 4, sloped edges are inconsistently steppable based on speed and move fraction when stepping
			// thats kinda stupid lol
			if (MaxStepHeight > 0 && abs(hit.Normal.z) < .001)
			{
				stepped = Movable_StepUp(hit, remaining, moveHits);
			}
			else if (hit.Normal.z > -.001)
			{
				// check for a tiny step
				// its possible to get snagged on a plane between brushes that isnt actually protruding
				// geometrically there is nothing blocking, and nothing to step up
				// but treating it like a regular step with small movements can get us over it
				MovableFlags |= MF_TinyStep;
				stepped = Movable_StepUp(hit, remaining, moveHits);
				MovableFlags &= ~MF_TinyStep;
			}
			
			// StepUp returns -1 if we died
			if (stepped == -1) return -1;

			bool pendingBlock = MovableFlags & MF_PendingMoveBlock;
			MovableFlags &= ~MF_PendingMoveBlock;

			if (stepped)
			{
				// call hit event if it happened
				if (pendingBlock)
				{
					OnMoveBlocked(PendingMoveBlock.PreVelocity, PendingMoveBlock.Hit);
					// leave this function if the OnMoveBlock override resulted in destruction
					if (!entRef.IsValid() || !MovementIsEnabled())
						return -1;
				}
				continue;
			}
		}

		GVec3 preClipVel = Velocity;
		GVec3 preNorm = hit.Normal;

		// move along whatever plane we hit
		if (IsWalking())
		{
			hit.Normal.z = 0;
			hit.Normal.Normalize();
		}
		if (!Movable_Bounce(hit.Normal))
		{
			GVec3 preVel = Velocity;
			ClipVector(Velocity, hit.Normal, Velocity);
			if (Velocity.z > preVel.z && hit.Normal.z > 0)
			{
				GFlt change = Velocity.z - preVel.z;
				change *= 1.5;
				Velocity.z = preVel.z + change;
			}
		}

		hit.Normal = preNorm;
		if (!stepMove)
		{
			// leave this function if the OnMoveBlock override resulted in destruction
			OnMoveBlocked(preClipVel, hit);
			if (!entRef.IsValid() || !MovementIsEnabled())
				return -1;
		}
		else
		{
			PendingMoveBlock.PreVelocity = preClipVel;
			PendingMoveBlock.Hit = hit;
			MovableFlags |= MF_PendingMoveBlock;
		}
	};

	if (!stepMove && resolves == 0)
	{
		// probably stuck
		GFlt z = Velocity.z;
		Velocity = 0;
		Velocity.z = z;
	}

	if (stepMove)
	{
		// move back down
		GHitResult stepHit;
		col->InitHitResult(stepHit);
		GFlt stepDown = tinyStep ? TINY_STEP_MOVE * 5 + 1 : MaxStepHeight + 1;
		u8 res = 0;
		Movable_MoveTrace(ent->GetPosition(), ent->GetPosition().AdjustZ(-stepDown), stepHit, res);

		if (!entRef.IsValid() || !MovementIsEnabled())
			return -1;

		if (stepHit.Time < 1 && (IsPhysics() || IsWalkable(stepHit.Normal)) )
		{
			// successful step
			MovableFlags &= ~MF_TestingStepUp;
			ent->SetPosition(stepHit.Point);
			return remaining;
		}
	}

	return remainingMove;
}

i32 KEntProp_Movable::Movable_StepUp(const GHitResult& hit, GFlt& remaining, MoveHitNormals& moveHits)
{
	if (IsFalling() && Velocity.GetNormalized().Dot(-hit.Normal) < .1)
		return false;

	KEntProp_CollidableBBox* col = Movable_GetCollision();
	KEntity* ent = GetEntity();

	bool tinyStep = MovableFlags & MF_TinyStep;

	MovableFlags |= MF_TestingStepUp;
	GVec3 preVel = Velocity;

	// move up as much as we can before going forward again
	GHitResult stepHit;
	col->InitHitResult(stepHit);
	GVec3 traceEnd = hit.Point;
	traceEnd.z += tinyStep ? TINY_STEP_MOVE * 5 : (Movable_GetMaxStepHeight() + .1);
	GPlane hitPlane(hit.Normal, hit.PlaneDistance);

	// ignore the plane that sent us here and any entities we've already hit
	const auto planeTest = [](KTraceHitParams& hits, KPlaneTestLocalCapture& locals) -> bool
	{
		if (locals.Collidable && hits.Test->Object == locals.Collidable) return false;

		if (VectorContains(locals.Movable->ThisFrameHitEntities, hits.Test->Object))
			return false;
		
		return (locals.Collidable->GetCollisionBlocks() & hits.Test->HitCollision)
			&& GPlane(hits.Test->Normal, hits.Test->PlaneDistance) != *locals.Plane;
	};

	KPlaneTestLocalCapture cap;
	cap.Movable = this;
	cap.Collidable = col;
	cap.Plane = &hitPlane;

	KFunctionArg<KTraceHitParams, KPlaneTestLocalCapture, bool> func(planeTest, cap);

	// trace up
	TraceBox(GLineSegment(hit.Point, traceEnd), col->GetCollisionBoundsHalfExtent(), stepHit, &func);

	GFlt preRemaining = remaining;
	if (stepHit.Time > .01) // do a regular move starting from raised position
	{
		ent->SetPosition(stepHit.Point);
		remaining = Movable_MoveAgainstCollision(remaining);
		if (remaining == -1) return -1;
	}

	if ((MovableFlags & MF_TestingStepUp) || ent->GetPosition().z <= hit.Point.z + .0001)
	{
		// if this flag is still set, the step was unsuccessful
		// if new z is <= pre z, we didnt actually step up
		// return to previous position and velocity
		MovableFlags &= ~MF_TestingStepUp;
		ent->SetPosition(hit.Point);
		Velocity = preVel;
		remaining = preRemaining;
	}
	else
	{
		// successful step

		if (!IsReplayingMoves())
			LastMoveStepDistance += ent->GetPosition().z - hit.Point.z;

		if (IsGroundMovement()) Velocity = preVel.SetZ(0);
		else Velocity = preVel;
		moveHits.RevertResolve();
		return true;
	}

	return false;
}

void KEntProp_Movable::Movable_DoJump()
{
	KMoveState_Walking* walk = GetWalkingMovement();
	if (!walk) return;
	GFlt jump = GetJumpVelocity(walk);

	// use current or last floor norm, take the steeper one
	const GVec3& floorNorm = walk->GetSteepestNormal();

	if (floorNorm.z < 1 && Velocity.Dot(floorNorm) < 0)
	{
#if 1
		GVec3 velRight = Velocity.Cross(GVec3(0, 0, 1));
		GVec3 upRamp = floorNorm.Cross(velRight);
		upRamp.z *= Velocity.Length() / upRamp.SetZ(0).Length();
		if (upRamp.z == upRamp.z) // NaN check
			jump = KClamp(jump + upRamp.z, jump, jump * 1.7);
#else
		// add height if we're walking up a slope
		GVec3 vel = Velocity;
		const GFlt angle = acos(floorNorm.z);
		const GFlt adjacent = vel.Normalize(); // normalize in place and return original length
		const GFlt opposite = tan(angle) * adjacent;

		const GVec3 up(0, 0, 1);
		const GVec3 normCross = floorNorm.Cross(up);
		const GVec3 velCross = vel.Cross(up);

		const GFlt into = normCross.GetNormalized().Dot(velCross.GetNormalized());
		jump = KClamp(jump + (opposite * abs(into)), jump, jump * 1.7);
#endif
	}

	/*flt_t velZ = GetEntity()->GetPosition().z - LastFramePosition.z;
	velZ /= GameFrameDelta();
	LOG(velZ);
	jump = KClamp(jump + velZ, jump, jump * 1.7);*/

	GFlt speedSq = std::pow(MovementProperties.MoveSpeed, 2);
	Velocity.z = 0;
	if (Velocity.LengthSq() > speedSq)
	{
		// apply half friction before jumping
		GFlt halfDecel = 1 - ((1 - MovementProperties.Deceleration) * .5);
		Velocity *= halfDecel;
		// dont get slower than run speed
		if (Velocity.LengthSq() < std::pow(MovementProperties.MoveSpeed, 2))
			Velocity = Velocity.GetNormalized() * MovementProperties.MoveSpeed;
	}

	Velocity.z = jump;
	SetMovementState(EMoveState::Falling);
	
	if (!IsReplayingMoves())
	{
		KEntity* ent = GetEntity();
		KSoundProperties props;
		props.MaxDistance = 1536;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		if (KSnapshottable* snap = ent->As<KSnapshottable>())
			props.PlayerIndex =	snap->OwningPlayerIndex;
		KAudio::PlaySoundAttached(KSoundID::Player_Jump, ent, props);
	}
}

void KEntProp_Movable::Movable_UpdateCrouchState(bool wantsCrouch)
{
	if (!wantsCrouch)
	{
		bool canStand = true;
		if (IsCrouching()) canStand = Movable_CanExitCrouch();
		if (canStand) MovableFlags &= ~MF_IsCrouching;
	}
	else
	{
		MovableFlags |= MF_IsCrouching;
	}
}

bool KEntProp_Movable::Movable_CanExitCrouch()
{
	KEntity* ent = GetEntity();

	if (KEntProp_CollidableBBox* box = ent->As<KEntProp_CollidableBBox>())
	{
		if (box->IsCollisionEnabled())
		{
			GHitResult hit;
			box->InitHitResult(hit, true, false);
			hit.PositionOffsetZ = 0;

			GVec3 testPos = ent->GetPosition();
			//testPos.z += GetCrouchDepth() * 16;

			TraceBox(GLineSegment(testPos, testPos), box->GetDefaultHalfExtent(), hit);
			if (hit.bHit)
			{
				// we can not stand up
				return false;
			}
		}
	}

	return true;
}

bool KEntProp_Movable::IsWalkable(const GVec3& norm)
{
	if (auto s = GetWalkingMovement())
		return cos(glm::radians(s->WalkableFloorAngle)) <= norm.z;
	return false;
}

bool KEntProp_Movable::IsCrouching() const
{
	return MovableFlags & MF_IsCrouching;
}

void KEntProp_Movable::SetMovementEnabled(bool enabled)
{
	if (enabled) MovableFlags |= MF_Enabled;
	else		 MovableFlags &= ~MF_Enabled;
}

void KEntProp_Movable::AddPendingImpulse(const GVec3& vel)
{
	PendingImpulse += vel;
}

GFlt KEntProp_Movable::GetJumpVelocity(KMoveState_Walking* walk)
{
	if (!walk) walk = GetWalkingMovement();
	if (walk)
		return sqrt(2 * GetGravityZ() * walk->JumpHeight);

	return 0;
}

GFlt KEntProp_Movable::GetGravityZ()
{
	return GetGameMatch()->GetWorldGravity() * GravityScale;
}

f32 KEntProp_Movable::GetCrouchDepth() const
{
	return LerpFade(KSaturate(CrouchDepth / f32(CROUCH_FRAMES)));
}

bool KEntProp_Movable::ShouldMoveThisFrame()
{
	KEntity* ent = GetEntity();
	return 
		(ent->Poolable_GetID() % FrameTickInterval) == (KTime::FrameCount() % FrameTickInterval);
}

void KEntProp_Movable::Push(GVec3 impulse, u32 pushFrames /*= 5*/)
{
	if (CanBePushed())
	{
		PushFramesRemaining = KMax<u32>(pushFrames, PushFramesRemaining);
		PendingImpulse += impulse;
	}
}

void KEntProp_Movable::SetSkipGroundCheck(bool skip)
{
	if (skip) MovableFlags |= MF_SkipGroundTrace;
	else	  MovableFlags &= ~MF_SkipGroundTrace;
}

void KEntProp_Movable::SetSimpleMovement(bool simple)
{
	if (simple) MovableFlags |= MF_IsSimpleMovement;
	else	    MovableFlags &= ~MF_IsSimpleMovement;
}

void KEntProp_Movable::SetBlockOnContact(bool block)
{
	if (block) MovableFlags |= MF_BlockOnContact;
	else	   MovableFlags &= ~MF_BlockOnContact;
}

bool KEntProp_Movable::PerformMovement(const GVec3& input)
{
	/*if (!IsNetAuthority())
	{
		if (KEntity_Character* cont = dynamic_cast<KEntity_Character*>(this))
		{
			if (!cont->IsControlledEntity())
			{
				cont->UpdateOccupiedCells();
				return true;
			}
		}
	}*/

	if (!ShouldMoveThisFrame())
	{
		FramesSinceMove++;
		return true;
	}
	else
		FramesSinceMove = 0;

	ThisFrameHitEntities.clear();
	LastMoveStepDistance = 0;
	MovableFlags &= ~MF_Teleported;

	KMoveState_Walking* walk = GetWalkingMovement();
	if (walk)
	{
		walk->CurrentFrameSteepestWalkingFloorNormal 
			= walk->WalkingFloorNormal;
	}

	if (MovementIsEnabled())
	{
		Movable_UpdateVelocity(Movable_ProcessInput(input));

		KEntProp_CollidableBBox* col = Movable_GetCollision();
		if (col && CrouchDepth > 0) col->CrouchDistance = GetCrouchDepth() * GetWalkingMovement()->CrouchDropDistance;

		LastWaterPenetration = WaterPenetration;
		WaterPenetration = -1;

		// returns -1 if entity was destroyed or movement was disabled during move
		if (Movable_MoveAgainstCollision() != -1)
		{
			if (!Movable_GroundCheck()) 
				return false;
		}
		else return false;
		
		if (col) col->UpdateOccupiedCells();
	}

	if (walk)
	{
		walk->LastFrameSteepestWalkingFloorNormal = 
			walk->CurrentFrameSteepestWalkingFloorNormal;
		walk->CurrentFrameSteepestWalkingFloorNormal = 0;
	}

	if (GetLocalPlayer()->ControlledEntity == GetEntity())
		GetLocalPlayer()->WaterDepth = WaterPenetration;

	return true;
}

KTestMovementResult KEntProp_Movable::TestMovement()
{
	return KTestMovementResult();
}

void KEntProp_Movable::SetCanBePushed(bool v)
{
	if (v)	MovableFlags |= MF_CanBePushed;
	else	MovableFlags &= ~MF_CanBePushed;
}

void KEntProp_Movable::ExplodePush(const GVec3& dir, f32 strength)
{
	if (!MovementIsEnabled()) return;
	Push(dir * strength, 10);
}

void KEntProp_Movable::CreateSavedMove()
{
#if !_SERVER
	if (!IsNetClient() || GetEntity() != GetControlledEntity()) return;
	
	const u8 state = GetGameInput()->GetState();
	const u32 frame = KTime::FrameCount();
	KMoveState_Walking* walk = GetWalkingMovement();
	KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(this);

	KSavedMove& move = SavedMoves[frame];

	move.Input = state;
	move.Position = GetEntity()->GetPosition();
	move.Velocity = Velocity;
	move.FloorNormal = walk ? walk->WalkingFloorNormal : 0;
	if (cont)
	{
		GFlt rad = glm::radians(GFlt(90));
		move.Pitch = std::round(MapRange(cont->GetPitch(), -rad, rad, 0, MAX_I16));
		rad = glm::radians(GFlt(180));
		move.Yaw = std::round(MapRange(cont->GetYaw(), -rad, rad, 0, MAX_U16));
	}

	if (KNetPlayer* p = GetLocalNetPlayer())
	{
		// delete frames up to last ack
		MapRemoveTo_Exclusive(SavedMoves, p->LastStateFrameUnpacked);
	}
#endif
}

void KEntProp_Movable::CheckServerMove(u32 frame, const GVec3& velocity, const GVec3& position, u16 pitch, u16 yaw)
{
#if !_SERVER
	if (!SavedMoves.contains(frame)) 
		return;

	if (!position.Equals(SavedMoves[frame].Position, .01)) // TODO make tolerance configurable
	{
		KEntity* ent = GetEntity();

		KSavedMove& move = SavedMoves[frame];

		// important: we have NOT yet simulated this frame, net stuff happens before entity ticks
		u32 current = KTime::FrameCount();

		KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(this);

		GFlt pitch = KInputView::MapPitchToFloat(move.Pitch);
		GFlt yaw = KInputView::MapYawToFloat(move.Yaw);
		u8 input = move.Input;

		GVec3 camStart = ent->GetPosition();

		if (cont)
		{
			cont->Controllable_SetInputFromState(input);
			cont->SetPitch(pitch, false);
			cont->SetYaw(yaw, false);
		}

		Velocity = velocity;
		ent->SetPosition(position);

		if (IsWalking()) SetMovementState(EMoveState::Falling);
		Movable_GroundCheck();
		
		if (current > frame)
		{
			for (frame++; frame < current; frame++)
			{
				if (!SavedMoves.contains(frame)) break;

				move = SavedMoves[frame];

				pitch = KInputView::MapPitchToFloat(move.Pitch);
				yaw = KInputView::MapYawToFloat(move.Yaw);
				input = move.Input;

				if (cont)
				{
					cont->Controllable_SetInputFromState(input);
					cont->SetPitch(pitch, false);
					cont->SetYaw(yaw, false);
				}

				PerformMovement(cont ? cont->GetInputVectorForMove() : 0);

				// update this frame in case it gets played again
				SavedMoves[frame].Position = ent->GetPosition();
				SavedMoves[frame].Velocity = Velocity;
			}
		}
		else
		{
			ent->SetPosition(move.Position);
			Velocity = move.Velocity;
		}

		GetLocalPlayer()->CameraCorrectionOffset += camStart - ent->GetPosition();
	}
#endif
}

void KEntProp_Movable::MoveHitNormals::ResetResolve()
{
	Resolve.AddedNorm = false;
	Resolve.AddedRepeat = false;
}

void KEntProp_Movable::MoveHitNormals::RevertResolve()
{
	if (Resolve.AddedNorm)
	{
		HitNorms[NormCount] = 0;
		NormCount--;
	}
	if (Resolve.AddedRepeat)
	{
		RepeatNorms[RepeatCount] = 0;
		RepeatCount--;
	}
}

bool KEntProp_Movable::MoveHitNormals::Update(const GVec3& norm, KEntProp_Movable* mover)
{
	ResetResolve();
	u8 i;
	for (i = 0; i < KMin<u8>(NormCount, 8); i++)
	{
		if (norm == HitNorms[i])
		{
			if (RepeatNorms[0] != norm && RepeatNorms[1] != norm)
			{
				RepeatNorms[RepeatCount % 2] = norm;
				Resolve.AddedRepeat = true;
				RepeatCount++;
			}

			mover->Velocity += (mover->IsWalking() ? norm.SetZ(0) : norm) * 1;
			//mover->GetEntity()->SetPosition(mover->GetEntity()->GetPosition() + norm);
			break;
		}
	}

	// if we've hit two separate planes more than one time each, move velocity away from them
	if (RepeatCount >= 2)
	{
		GVec3 cross = RepeatNorms[0] ^ RepeatNorms[1];
		mover->Velocity *= cross.GetNormalized().Abs();
		return false;
	}

	if (i < NormCount)
	{
		mover->ClipVector(
			mover->Velocity, 
			mover->IsWalking() ? norm.SetZ(0) : norm, 
			mover->Velocity);
		return false;
	}

	// new plane, add it
	HitNorms[NormCount % 8] = norm;
	Resolve.AddedNorm = true;
	NormCount++;
	return true;
}

bool KEntProp_Movable::OnOverlapChannel(u32 channel, const GVec3& preVel, GHitResult& hit)
{
	ThisFrameHitEntities.push_back(hit.Object);

	switch (channel)
	{
		case ECollisionMask::WorldStatic: 
			return OnOverlapWorldStatic(preVel, hit);
		case ECollisionMask::Light: 
			return OnOverlapLight(preVel, hit);
		case ECollisionMask::Precipitation: 
			return OnOverlapPrecipitation(preVel, hit);
		case ECollisionMask::Portal: 
			return OnOverlapPortal(preVel, hit);
		case ECollisionMask::Launcher: 
			return OnOverlapLauncher(preVel, hit);
		case ECollisionMask::Water: 
			return OnOverlapWater(preVel, hit);
		case ECollisionMask::Damage: 
			return OnOverlapDamage(preVel, hit);
		case ECollisionMask::PlayerCharacter: 
			return OnOverlapPlayerCharacter(preVel, hit);
		case ECollisionMask::MonsterCharacter: 
			return OnOverlapMonsterCharacter(preVel, hit);
		case ECollisionMask::Gib: 
			return OnOverlapGib(preVel, hit);
		case ECollisionMask::Pickup: 
			return OnOverlapPickup(preVel, hit);
		case ECollisionMask::Weapon: 
			return OnOverlapWeapon(preVel, hit);
	}
	return false;
}

bool KEntProp_Movable::OnOverlapWorldStatic(const GVec3& preVel, GHitResult& hit)
{
	return true;
}

bool KEntProp_Movable::OnOverlapLight(const GVec3& preVel, GHitResult& hit)
{
	return true;
}

bool KEntProp_Movable::OnOverlapPrecipitation(const GVec3& preVel, GHitResult& hit)
{
	return true;
}

bool KEntProp_Movable::OnOverlapPortal(const GVec3& preVel, GHitResult& hit)
{
	GetEntity()->SetPosition(hit.Point);

	if (KMapEntity_Portal* p = dynamic_cast<KMapEntity_Portal*>(hit.GetMapEntity()))
	{
		KEntProp_CollidableBBox* col = const_cast<KEntProp_CollidableBBox*>(hit.BoxEntity);

		if (p->Target)
		{
			MovableFlags |= MF_Teleported;

			GVec3 prePos = GetEntity()->GetPosition();
#if !_SERVER
			KEntity_PortalTravel::Create(prePos, col->GetCollisionBoundsHalfExtent(), true);
			KEntity_PortalTravel::Create(p->Target->GetMapPosition(), col->GetCollisionBoundsHalfExtent(), false);
#endif
			GetEntity()->SetPosition(p->Target->GetMapPosition().AdjustZ(.01));
			GFlt angle = glm::radians(p->Target->GetAngle());

			if (p->bResetVelocity)
			{
				Velocity = 0;
			}
			else
			{
				GVec3 dir = GVec3::FromPitchYaw(0, angle);

				// rotate to match entry direction
				if (ForceMaintainPortalRotation())
				{
					GFlt entryAngle = Velocity.GetNormalized().AngleBetween(-hit.Normal);
					GVec3 entryUp = abs(hit.Normal.z) > .99 ? GVec3(1, 0, 0) : GVec3(0, 0, 1);
					GVec3 entryRight = -hit.Normal.Cross(entryUp);
					if (entryRight.Dot(Velocity) > 0) entryAngle = -entryAngle;
					dir.Rotate(entryAngle, entryUp);
				}

				Velocity = dir * Velocity.Length();
			}

			if (KEntProp_Controllable* control = dynamic_cast<KEntProp_Controllable*>(this))
			{
				control->SetYaw(angle);
				control->SetPitch(0);
			}

			col->AttemptTelefrag();

			if (GetEntity()->As<KEntity_Character>())
			{
				if (GetEntity() == GetLocalPlayer()->ControlledEntity.Get())
				{
					KAudio::PlaySound(KSoundID::Enter_Portal);
					GetLocalPlayer()->ShakeCamera(.75, .75, 30);
#if !_SERVER
					KGameInstance::Get().LastTeleportTime = KGameInstance::Get().GetTotalRenderTime();
#endif

					if (IsNetServer())
					{
						// we are hosting and still need to replicate this sound
						KSoundProperties props;
						props.ReplicationMethod = EReplicatedSound::SendAll;
						KAudio::PlaySound3D(KSoundID::Enter_Portal, p->Target->GetMapPosition(), props);
					}
				}
				else
				{
					KSoundProperties props;
					if (KNetPlayer* player = GetEntity()->GetOwningPlayer())
					{
						props.ReplicationMethod = EReplicatedSound::SkipIndex;
						props.PlayerIndex = player->OwningPlayerIndex;
					}
					KAudio::PlaySound3D(KSoundID::Enter_Portal, p->Target->GetMapPosition(), props);
				}
			}

			if (!Movable_GroundCheck()) return false;
		}
	}

	return false;
}

bool KEntProp_Movable::OnOverlapLauncher(const GVec3& preVel, GHitResult& hit)
{
	GetEntity()->SetPosition(hit.Point);
	KMapEntity_Launcher* launcher = dynamic_cast<KMapEntity_Launcher*>(hit.GetMapEntity());
	Velocity = launcher->GetLaunchVelocity(GetEntity()->GetPosition());
	if (IsGroundMovement()) SetMovementState(EMoveState::Falling);
	return false;
}

bool KEntProp_Movable::OnOverlapWater(const GVec3& preVel, GHitResult& hit)
{
	GetEntity()->SetPosition(hit.Point);
	return false;
}

bool KEntProp_Movable::OnOverlapDamage(const GVec3& preVel, GHitResult& hit)
{
	GetEntity()->SetPosition(hit.Point);

	if (IsNetAuthority())
	  if (KEntProp_Killable* k = GetEntity()->As<KEntProp_Killable>())
	    k->SetHealth(0);

	return false;
}

bool KEntProp_Movable::OnOverlapPlayerCharacter(const GVec3& preVel, GHitResult& hit)
{
	return true;
}

bool KEntProp_Movable::OnOverlapMonsterCharacter(const GVec3& preVel, GHitResult& hit)
{
	return true;
}

bool KEntProp_Movable::OnOverlapGib(const GVec3& preVel, GHitResult& hit)
{
#if !_SERVER
	KEntity_GibBase* gib = dynamic_cast<KEntity_GibBase*>(((KEntProp_CollidableBBox*)hit.Object)->GetEntity());
	gib->SetMovementEnabled(true);
	gib->Velocity = GetGibPushVelocity(gib->Velocity);
	GetEntity()->SetPosition(hit.Point);
	return false;
#else
	return true;
#endif
}

bool KEntProp_Movable::OnOverlapPickup(const GVec3& preVel, GHitResult& hit)
{
	KEntity_PickupBase* pickup = dynamic_cast<KEntity_PickupBase*>(((KEntProp_CollidableBBox*)hit.Object)->GetEntity());
	GetEntity()->SetPosition(hit.Point);

	if (pickup->IsDropItem())
	{
		pickup->SetMovementEnabled(true);
		pickup->Velocity = GetItemPushVelocity(pickup->Velocity);
	}
	pickup->OnOverlap(GetEntity());

	return false;
}

bool KEntProp_Movable::OnOverlapWeapon(const GVec3& preVel, GHitResult& hit)
{
	KEntity_Projectile* proj = dynamic_cast<KEntity_Projectile*>(((KEntProp_CollidableBBox*)hit.Object)->GetEntity());
	GetEntity()->SetPosition(hit.Point);

	GHitResult h;
	h.HitCollision = hit.BoxEntity->GetCollisionChannels();
	h.Normal = -hit.Normal;
	h.Point = proj->GetPosition();
	h.Object = const_cast<KEntProp_CollidableBBox*>(hit.BoxEntity);
	proj->OnOverlapChannel(h.HitCollision, proj->Velocity, h);

	return false;
}
