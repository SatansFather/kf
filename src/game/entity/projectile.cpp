#include "projectile.h"
#include "character_player.h"

#if !_SERVER
#include "smoke.h"
#include "graphics/hit_spark.h"
#include "engine/render/interface/render_interface.h"
#include "graphics/water_splash.h"
#include "graphics/bullet_hole.h"
#endif
#include "engine/utility/random.h"
#include "engine/collision/trace.h"
#include "engine/net/player.h"
#include "engine/collision/broadphase/trace_args.h"
#include "../../engine/game/local_player.h"
#include "../../engine/net/net_interface.h"
#include "../../engine/net/state.h"

KEntity_Projectile::KEntity_Projectile()
{
	PhysicsProperties.Deceleration = .92;
	PhysicsFluidProperties.Deceleration = 1;
	SinkSpeed = 0;
	SetMovementState(EMoveState::Physics);
	SetCollisionBoundsHalfExtent(GVec3(4, 4, 4));
	MaxStepHeight = 4;
	GravityScale = 0;
	CollisionChannels = ECollisionMask::Weapon;
	CollisionBlocks |= ECollisionMask::WorldStatic;
	CollisionOverlaps |= ECollisionMask::PlayerCharacter;
	CollisionOverlaps |= ECollisionMask::MonsterCharacter;
	CollisionBlocks |= ECollisionMask::Portal;
	//CollisionOverlaps |= ECollisionMask::Launcher;
	CollisionOverlaps |= ECollisionMask::Gib;
	CollisionOverlaps |= ECollisionMask::Pickup;
	CollisionOverlaps |= ECollisionMask::Water;
#if !_SERVER
	RandomSeed = RandFloat(0, 3);
#endif
	//DisableCollision();

	SetCanBeDamaged(false);
	SetCanBePushed(false);

	//SetSimpleMovement(true);
	SetSkipGroundCheck(true);
	SetBlockOnContact(true);
}

void KEntity_Projectile::OnMoveBlocked(const GVec3& preVel, const GHitResult& hit)
{
	OnProjectileHit(preVel, hit);
}

void KEntity_Projectile::OnEnteredWater(const GHitResult& hit)
{
#if !_SERVER
	KEntity_WaterSplash::Create(hit.Point - hit.Normal, hit.Normal, FColor32(.72, .52, .68, .35).To8(), 5, 24);
#endif
}

void KEntity_Projectile::Tick()
{	
	if (IsMyProjectile() && GetEntityAge() >= Lifespan)
	{
		DestroyEntity();
		return;
	}

	//if (NetType == ENetProjectileType::ServerSweepPre && IsNetServer())
	//  if (KNetPlayer* p = GetOwningPlayer())
	//	if (!PingSweep(KTime::FrameCount() - p->LastStateFrame)) return;

	i32 frames = 1;

	// if this is our local projectile, it is simulating ping comp
	// tick once for every snapshot frame that was advanced this frame
	if (IsNetClient() && IsMyProjectile() && !IsMyNetProjectile())
	{
		frames = GetNetState()->SnapshotsProcessedThisFrame;
		PreClientLocalTick(frames);
	}

	TObjRef<KEntity_Projectile> thisProj = this;
	for (frames; frames > 0 && thisProj.IsValid(); frames--)
	{
		if (!PerformMovement(0)) return;

		LastRenderOffset = RenderOffset;
		RenderOffset /= RenderOffsetAdjustRate;

		ProjectileTick();
	}
}

bool KEntity_Projectile::PingSweep(u32 frames)
{
	if (NetType != ENetProjectileType::ServerSweepPre) return true;

	// do this now to prevent recursively calling this in Tick()
	NetType = ENetProjectileType::ServerSweepPost;

	u32 currentFrame = KTime::FrameCount();
	u32 startFrame = currentFrame >= frames ? currentFrame - frames : 0;

	TObjRef<KEntity_Projectile> entRef = this;

	for (u32 frame = startFrame; frame < currentFrame; frame++)
	{
		// adjust lifespan each frame
		if (EntityFrameCreated > 0) EntityFrameCreated--;

		// TODO adjust collision scene before tick
		// create an object that adjusts it, and resets it in destructor

		// OR just let entity collision cells store their history and test against that
		// ^^ if i do that then this function is useless, no need to sweep because checking 
		//		against old collision every frame doesnt cost extra because nothing needs to be moved

		Tick();

		// we may have died during the move
		if (!entRef.IsValid())
		{
			// "this" is dead, do not access members
			// for now, projectiles handle destruction of their own cosmetic pair
			return false;
		}	
	}
	return true;
}

bool KEntity_Projectile::IsSweepProjectile() const
{
	return NetType == ENetProjectileType::ServerSweepPost 
		|| NetType == ENetProjectileType::ServerSweepPre;
}

void KEntity_Projectile::InitNetObject()
{
	DisableCollision();
	SetPosition(ReplicatedPosition.ToVec3());
	Velocity = ReplicatedVelocity.ToVec3();
}

void KEntity_Projectile::PreCreateSnapshot()
{
	ReplicatedVelocity = Velocity;
	ReplicatedPosition = GetPosition();
}

void KEntity_Projectile::InitBaseProjectile(const KProjectileCreationParams& params, KEntity_Projectile* proj)
{
	proj->SetPosition(params.Position);
	proj->Velocity = params.Direction * params.Speed;
	proj->RenderOffset = params.RenderOffset;
	proj->bPrimaryFire = params.bPrimaryFire;

	if (KSnapshottable* snap = proj->As<KSnapshottable>())
	{
		snap->OwningPlayerIndex = params.OwningPlayerIndex;
		
		if (IsNetServer())
		  if (KNetPlayer* player = proj->GetOwningPlayer())
			if (player != GetLocalNetPlayer())
			  proj->CompFrames = player->CurrentCompFrames;
	}

	if (params.FiringWeapon)
	{
		proj->FiringWeapon = params.FiringWeapon;
		KEntity* guy = params.FiringWeapon->GetCarryingEntity();
		if (KEntProp_CollidableBBox* box = dynamic_cast<KEntProp_CollidableBBox*>(guy))
			proj->AssignIgnoreID(box->IgnoreID);
	}
}


bool KEntity_Projectile::IsMyProjectile()
{
	return HasObjectAuthority();
}

bool KEntity_Projectile::IsMyNetProjectile()
{
	return IsNetClient() && NetID != 0 && OwningPlayerIndex == GetLocalPlayer()->NetPlayerIndex;
}

void KEntity_Projectile::SplashDamage(const GHitResult& hit, const GVec3& preVel, GFlt radius, GFlt maxDamage, GFlt splashPushScale, GFlt selfDamageScale)
{
	const GVec3& origin = hit.Point;
	const GVec3& normal = hit.Normal;

	GBoundingBox thisBox = GetAdjustedBounds();

	KEntity* carrier = FiringWeapon.Get() ? FiringWeapon.Get()->GetCarryingEntity() : nullptr;

	TObjRef<KEntity> hitguy;

	KEntProp_CollidableBBox* col = nullptr;
	if (hit.HitCollision >= ECollisionMask::PlayerCharacter)
	{
		col = (KEntProp_CollidableBBox*)hit.Object;
		//u32 owner = col->GetEntity()->GetOwningPlayer()->OwningPlayerIndex;
		hitguy = col->GetEntity();
	}

	const auto splashDamage = [](KEntity*& ent, KProjectileExplodeLocalCapture& locals) -> bool
	{
		if (ent == locals.Proj) return false;

		bool comped = ent->GetOwningPlayer() != locals.Proj->GetOwningPlayer();
		KEntProp_CollidableBBox* box = ent->As<KEntProp_CollidableBBox>();
		GVec3 entPos = ent->GetPosition();

		if (comped) entPos += ent->GetCompedPositionOffset();
		const GFlt radius = locals.Radius;
		if (entPos.DistanceSq(*locals.Origin) > pow(radius + box->GetLargestDimension(), 2))
			return false;

		GBoundingBox entBox = box->GetAdjustedBounds();
		if (comped)
		{
			entBox.Min += ent->GetCompedPositionOffset();
			entBox.Max += ent->GetCompedPositionOffset();
		}

		// line trace to the nearest point on the entity
		GHitResult hit;
		hit.SearchCollision |= ECollisionMask::WorldStatic;
		hit.TraceCollision |= ECollisionMask::Weapon;

		GVec3 nearest;
		entBox.ClosestPointOnBoxToExternalPoint(*locals.Origin, nearest);

		if (nearest.DistanceSq(*locals.Origin) > radius * radius)
			return false;

		bool blocked = false;
		if (TraceLine(GLineSegment(*locals.Origin, nearest), hit))
			blocked = true;

		if (blocked)
		{
			GFlt minComp = box->GetCollisionBoundsHalfExtent().MinComponent();
			GVec3 reflected = locals.PreVel->Reflect(*locals.Normal).GetNormalized();

			if (!TraceLine(GLineSegment(*locals.Origin - reflected * minComp, entPos), hit))
				goto unblocked;

			const GVec3 center = entBox.GetCenter();

			// iterate corners of bounds
			for (i32 x = 0; x < 2; x++)
			{
				for (i32 y = 0; y < 2; y++)
				{
					for (i32 z = 0; z < 2; z++)
					{
						const GFlt xx = entBox[x].x;
						const GFlt yy = entBox[y].y;
						const GFlt zz = entBox[z].z;

						GVec3 test(xx, yy, zz);
						test = GVec3::Lerp(test, center, .1);

						if (!TraceLine(GLineSegment(*locals.Origin - reflected * minComp, nearest), hit)) 
							goto unblocked;
					}
				}
			}
			return false; // escapes with goto if unblocked
		}

	unblocked: // sick goto dude

		bool direct = locals.Collision == box;

		GFlt dist = locals.ThisBox->DistanceToBox(entBox);

		if (locals.Carrier == ent) entPos.z += 20;

		GVec3 dir = (entPos - *locals.Origin).GetNormalized();

		GFlt strength = KSaturate(MapRange(dist, 0, locals.Radius, 1, 0));
		if (KEntProp_Movable* m = ent->As<KEntProp_Movable>())
		{
			GFlt scale = locals.Proj->PushScale * strength;
			if (!direct) scale *= locals.SplashPushScale;

			if (scale > 0)
				m->ExplodePush(dir, scale);
		}
		if (KEntProp_Killable* k = ent->As<KEntProp_Killable>())
		{
			GFlt damage = locals.MaxSplashDamage;
			if (locals.Carrier == ent) damage *= locals.SelfDamageScale;
			k->TakeDamage(std::round(strength * damage) + (locals.Collision == box ? locals.Proj->DirectDamage : 0), locals.Proj->GetOwningPlayer());
		}

		if (ent == locals.Carrier)
			locals.bHitCarrier = true;

		return false;
	};

	KProjectileExplodeLocalCapture cap;
	cap.Proj = this;
	cap.Collision = col;
	cap.Carrier = carrier;
	cap.ThisBox = &thisBox;
	cap.Normal = &normal;
	cap.PreVel = &preVel;
	cap.Origin = &origin;
	cap.MaxSplashDamage = maxDamage;
	cap.Radius = radius;
	cap.SplashPushScale = splashPushScale;
	cap.SelfDamageScale = selfDamageScale;

	KFunctionArg<KEntity*, KProjectileExplodeLocalCapture, bool> func(splashDamage, cap);

	PerformOnOverlapping(origin, GVec3(radius), &func, CompFrames);

	// good chance that the carrier wasnt found in the comped cell
	// try it manually
	if (carrier && !cap.bHitCarrier)
		splashDamage(carrier, cap);
}

bool KEntity_Projectile::OnOverlapPlayerCharacter(const GVec3& preVel, GHitResult& hit)
{
	OverlapCharacter(preVel, hit);
	return false;
}

bool KEntity_Projectile::OnOverlapMonsterCharacter(const GVec3& preVel, GHitResult& hit)
{
	OverlapCharacter(preVel, hit);
	return false;
}

void KEntity_Projectile::OverlapCharacter(const GVec3& preVel, GHitResult& hit)
{
	SetPosition(hit.Point);
	OnProjectileHit(preVel, hit);
}

GVec3 KEntity_Projectile::GetItemPushVelocity(const GVec3& otherVel)
{
	return GetGibPushVelocity(otherVel);
}

GVec3 KEntity_Projectile::GetGibPushVelocity(const GVec3& otherVel)
{
	const f32 maxSpeed = 1000;

	GVec3 otherLen = otherVel.LengthSq();
	GVec3 vel = Velocity.GetClampedToMaxLength(maxSpeed);
	GVec3 out = vel + otherVel;
	//if (otherLen.LengthSq() < 2000 * 2000)
	out = out.GetClampedToMaxLength(maxSpeed);

	return out;
}


/*
* 
* #include <iostream>
#include <cmath>

using namespace std;

const double g = 9.8; // acceleration due to gravity

struct Vector3D {
  float x, y, z;
};

// Function to calculate the launch velocity required to hit a target
// Arguments: target position (targetPos), target velocity (targetVel), and launch position (launchPos)
// Returns: launch velocity (v0)
Vector3D deadReckoning3D(Vector3D targetPos, Vector3D targetVel, Vector3D launchPos) {
  Vector3D result;

  float dx = targetPos.x - launchPos.x;
  float dy = targetPos.y - launchPos.y;
  float dz = targetPos.z - launchPos.z;
  float d = sqrt(dx * dx + dy * dy + dz * dz);
  float v0 = sqrt(g * d / sin(2 * asin(dz / d)));

  result.x = v0 * (targetPos.x - launchPos.x) / d;
  result.y = v0 * (targetPos.y - launchPos.y) / d;
  result.z = v0 * (targetPos.z - launchPos.z) / d;

  return result;
}

int main() {
  Vector3D targetPos, targetVel, launchPos;
  cout << "Enter the target position (tx, ty, tz): ";
  cin >> targetPos.x >> targetPos.y >> targetPos.z;
  cout << "Enter the target velocity (vx, vy, vz): ";
  cin >> targetVel.x >> targetVel.y >> targetVel.z;
  cout << "Enter the launch position (px, py, pz): ";
  cin >> launchPos.x >> launchPos.y >> launchPos.z;

  Vector3D v0 = deadReckoning3D(targetPos, targetVel, launchPos);

  cout << "The launch velocity required to hit the target is: (" << v0.x << ", " << v0.y << ", " << v0.z << ")" << endl;

  return 0;
}

This code calculates the launch velocity required to hit a target in 3D space, given the target position (targetPos), target velocity (targetVel), and the launch position of the projectile (launchPos), represented as vector structs. The calculated launch velocity is returned as a vector struct v0.
Chris Tryon
3d dead reckoning formula c++ function that takes 3 arguments: target position, target velocity, and projectile launch position; and returns the projectile launch velocity. vector types are a class containing float x y z components

Here's a code in C++ that implements a 3D dead reckoning function to calculate the launch velocity required
*/