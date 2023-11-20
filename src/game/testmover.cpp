/*
#include "testmover.h"
#include "engine/math/math.h"
#include "../engine/math/vec3.h"
#include "../engine/input/view.h"
#include "../engine/input/binding.h"
#include "../engine/game_instance.h"
#include "../engine/system/terminal/terminal.h"
#include "engine/collision/hit_result.h"
#include "../compiler/compiler.h"
#include "../compiler/bsp/bsp_tree.h"
#include "../engine/collision/broadphase/cell_coordinate.h"
#include "../engine/collision/broadphase/bvh_node.h"
#include "../engine/collision/broadphase/bvh_grid.h"
#include "../engine/input/listener_game.h"


static bool IsWalkable(const DVec3& normal)
{
	return normal.z >= .707;
}

f64 GravityScale = 1;
f64 GravityZ = 1150;
f64 JumpHeight = 40;

static f64 GetJumpVelocity() 
{
	return sqrt(2 * GravityZ * JumpHeight) / GravityScale;
}

static void UpdateFloor(KTestMover* mover)
{
	if (mover->bFlying) return;

	DHitResult dhit;
	
	KMapCompiler::Get().Grid->TraceBox(DLineSegment(mover->Position.ToType<f64>(), mover->Position.AdjustZ(-0.1).ToType<f64>()), DVec3(16, 16, 24), dhit);

	//KMapCompiler::Get().BspHead->TraceSphere(DLineSegment(mover->Position.ToType<f64>(), mover->Position.AdjustZ(-0.1).ToType<f64>()), 16, &dhit);
	//ShapeTraceFloor(hit, col->Shape.get(), owner->GetLocation(), owner->GetLocation().AdjustZ(-0.1), CC_World);

	DHitResult hit = dhit.ToType<f64>();

	bool ground = mover->bOnGround;
	mover->bOnGround = hit.bHit && IsWalkable(hit.Normal);
	if (mover->bOnGround)
	{
		mover->FloorNormal = hit.Normal.GetNormalized();
		mover->FloorPoint = hit.Point;

		DVec3 pos = mover->Position;
		pos.z = mover->FloorPoint.z;
		mover->Position = pos;
	}
	else
	{
		// step down
		if (!mover->bFlying && ground && mover->DesiredDirection.z < .01)
		{
			DHitResult hitter;
			KMapCompiler::Get().Grid->TraceBox(DLineSegment(mover->Position.ToType<f64>(), mover->Position.AdjustZ(-17).ToType<f64>()), DVec3(16, 16, 24), hitter);
			
			mover->bOnGround = hitter.bHit && IsWalkable(hitter.Normal);
			if (mover->bOnGround)
			{
				//auto cam = Camera.lock();
				//Vec3 pre_cam;
				//if (cam) pre_cam = cam->GetWorldLocation();

				mover->FloorNormal = hitter.Normal.GetNormalized();
				mover->FloorPoint = hitter.Point;
				//FloorObject = hit.Object;

				DVec3 pos = mover->Position;
				pos.z = mover->FloorPoint.z/ * + 24* /;
				mover->Position = pos;

				/ *if (cam && !bReplayingMoves)
				{
					cam->StepOffset += Vec3(0, 0, pre_cam.z - cam->GetWorldLocation().z);
					//cam->SetWorldLocation(pre_cam);
				}* /

				return;
			}
		}
		
		mover->FloorNormal = DVec3(0, 0, 0);
		mover->FloorPoint = DVec3(0, 0, 0);
		mover->OffGroundZ = mover->Position.z;
	}
}

static void UpdatePosition(DVec3 pos, KTestMover* mover)
{
	mover->Position = pos;
	UpdateFloor(mover);
}

KTestMover::KTestMover()
{
	//BIND_DELEGATE_THREADSAFE(KTestMover::UpdateKeyState, StateListener, KGameInstance::Get().GameInputBroadcaster->KeyStateBroadcaster);
}

void KTestMover::Tick(f64 delta)
{
	typedef DVec3 Vec3;

	KeyState = GetGameInput()->GetState();

	bFlying = KeyState & (u8)EInputKeyState::AltFiring;

	f32 pitch, yaw;
	KInputView::GetViewForGame(pitch, yaw);

	DesiredDirection = Vec3
	(
		((KeyState & (u8)EInputKeyState::MoveForward) ? 1 : 0) + ((KeyState & (u8)EInputKeyState::MoveBack) ? -1 : 0),
		((KeyState & (u8)EInputKeyState::MoveRight) ? -1 : 0) + ((KeyState & (u8)EInputKeyState::MoveLeft) ? 1 : 0),
		((KeyState & (u8)EInputKeyState::Jump) ? 1 : 0) + ((KeyState & (u8)EInputKeyState::Crouching) ? -1 : 0)
	);

	DesiredDirection.Rotate(glm::degrees(yaw), Vec3(0, 0, 1));
	DesiredDirection.Normalize();

	Vec3 start = Position;

	{
		Vec3 offset;
		{
			if (DesiredDirection.Length() < .001 && Vec3(Velocity.x, Velocity.y, bFlying ? Velocity.z : 0).Length() < 5)
			{
				offset = Vec3(0, 0, 0);
			}
			else
			{
				bool ice = false;

				f64 accel = bFlying ? 5.f : 7.f;
				f64 speed = bFlying ? 700 : Speed;

				if (ice) accel *= .15;

				Vec3 desired =		 // this is "correct" but feels bad
					(DesiredDirection/ *.SetZ(0).GetNormalized()* /) *

					//accel
					(   (RemainingPushFrames > 0 ? .1 : 1) *
						// knockback reduces accel and friction
						( (bOnGround || bFlying) ? speed * (1.f / .8f) : speed ) *
						( (bOnGround || bFlying) ? accel : 2.f )
					) *

					delta;

				Vec3 vel = Velocity;
				if (!bFlying) vel.z = 0;

				Vec3 temp = vel + desired;

				if (bOnGround || bFlying)
				{
					f64 decel = bFlying ? .92f : (ice ? .99f : .875f);

					f64 adjust = MapRange(RemainingPushFrames, 0, 3, decel, 1.f);
					adjust = std::clamp(adjust, decel, 1.0);
					temp *= adjust;
				}

				f64 templength = bFlying ? temp.Length() : temp.SetZ(0).Length();

				if (templength < .1f)
				{
					temp = Vec3(0, 0, 0);
				}
				else
				{
					f64 m = (std::max)((speed + 30), vel.Length());
					if (m <= speed + 30) m = speed;
					if (templength > m)
						temp = temp.GetNormalized() * m;
				}

				offset = temp;
			}
		}


		if (bFlying)
		{
			Velocity = offset;
		}
		else
		{
			f64 z = Velocity.z;
			Velocity = offset;
			Velocity.z = z;

			//SYSLOG(z);

			if (!bOnGround)
			{
				Velocity.z -= GravityZ * GravityScale * delta;
			}
			else
			{
				if (DesiredDirection.z > 0)
				{
					// jump
					f32 jump = GetJumpVelocity();

					Velocity.z += jump;
					f64 jumpvel = GetJumpVelocity();
					Velocity.z = std::clamp(Velocity.z, jumpvel, jumpvel * 1.7f);

					if (Velocity.z < jump) Velocity.z = jump;
				}
				else
				{
					// adjust z to move along floor
					Vec3 a = Velocity ^ Vec3(0, 0, 1);
					Vec3 b = FloorNormal ^ a;
					b.Normalize();
					Velocity = b * Velocity.Length();
				}
			}
		}
	}

	//Position += Velocity * delta;
	Vec3 trace_start = start;

	u8 max_resolves = 8;
	for (u8 i = 0; i < max_resolves; i++)
	{
		// TODO does this really need to be here?
		// maybe try to re-jump after a resolution (also maybe not because that could just force this to the max resolve count)
		/ *if (bOnGround && DesiredDirection.z <= 0)
		{
			// adjust z to move along floor
			Vec3 a = Velocity ^ Vec3(0, 0, 1);
			Vec3 b = FloorNormal ^ a;
			b.Normalize();
			Velocity = b * Velocity.Length();
		}* /

		Vec3 trace_end = trace_start + Velocity * delta;

		////////////
		//UpdatePosition(trace_end, this);
		//return;
		////////////

		f64 dist = trace_end.Distance(trace_start);

		DHitResult dhit;
		KMapCompiler::Get().Grid->TraceBox(DLineSegment(trace_start.ToType<f64>(), trace_end.ToType<f64>()), DVec3(16, 16, 24), dhit);



		DHitResult hit = dhit.ToType<f64>();

		if (i == 0)
		{
			HitPoint = hit.Point.AdjustZ(-4);
			HitNormal = hit.Normal;
		}

		// adjust target after finding our exact hit
		Vec3 adjusted_target = trace_start + (Velocity * delta * hit.Time);

		UpdatePosition(adjusted_target, this);

		if (false && hit.bHit)
		{
			const f64 step_height = 18;

			f32 step_dist = (trace_end - adjusted_target).Length();
			step_dist = (step_dist > 1.f) ? 1.f : step_dist;

			//Vec3 step_target = adjusted_target + ((trace_end - trace_start).GetNormalized() * step_dist);

			// scale shape 1 unit tall
			//col->Shape->setLocalScaling(BtFromVec3(Vec3(1, 1, .1f / 24.f)));

			// find highest step z based on owner position
			Vec3 feet = Position.AdjustZ(-24 + step_height);
			feet += Velocity.SetZ(0).GetNormalized() * step_dist;

			// first check if we're not already inside something
			{
				// TODO we cant run into wall + go up stairs because we need to redirect velocity (below) before moving up step

				/ *ContactTestCallbackMulti step_callback;
				{
					step_callback.m_collisionFilterGroup = CC_Character;
					step_callback.m_collisionFilterMask = CC_World;
					btTransform t = col->Object->getWorldTransform();
					t.setOrigin(BtFromVec3(feet));
					col->Object->setWorldTransform(t);
					Bullet->CollisionWorld->contactTest(col->Object.get(), step_callback);
					if (step_callback.Hits.size() > 0)
					{
						col->Shape->setLocalScaling(btVector3(1, 1, 1));
						goto no_steps;
					}
				}* /
			}

			DHitResult step_hit;

			//ShapeTraceFloor(step_hit, col->Shape.get(), feet, feet.AdjustZ(-step_height), CC_World);
			KMapCompiler::Get().Grid->TraceBox(DLineSegment(feet, feet.AdjustZ(-step_height)), DVec3(16, 16, 1), step_hit);

			if (step_hit.bHit && IsWalkable(step_hit.Normal))
			{
				if (feet.z - step_hit.Point.z < step_height - 1)
				{
					adjusted_target.z = step_hit.Point.z + 24;
					trace_start = adjusted_target;

					Velocity.z = 0;
					//bOnGround = true;
					//FloorNormal = step_hit.Normal;
					//FloorObject = step_hit.Object;
					//FloorPoint = step_hit.Point;

					/ *if (!bReplayingMoves)
					{
						if (auto cam = Camera.lock())
						{
							cam->StepOffset += Vec3(0, 0, -(step_height - (feet.z - step_hit.Point.z)));
						}
					}* /

					// velocity is unchanged, continue with move
					continue;
				}
			}
		}

no_steps:

		if (hit.Time == 1)
		{
			// if somehow there was no hit (despite contact result saying otherwise) update velocity and break
			Velocity = (adjusted_target - trace_start) / delta;
			break;
		}
		else if (hit.bHit)
		{
			// we didnt step, redirect velocity based on hit result and keep going
			Velocity += hit.Normal * -(hit.Normal | (Velocity));
			trace_start = adjusted_target;
		}
	}
}

void KTestMover::UpdateKeyState(u8 state)
{
	KeyState = state;
}

*/
