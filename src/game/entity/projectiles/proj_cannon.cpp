
#include "proj_cannon.h"
#include "engine/collision/trace.h"
#include "../weapon.h"
#include "../explosion.h"
#include "engine/utility/random.h"
#include "engine/render/interface/render_interface.h"
#include "engine/audio/sound_asset.h"
#include "engine/audio/audio.h"
#include "engine/net/player.h"
#include "engine/game_instance.h"
#include "../weapons/wep_cannon.h"

KEntity_Projectile_Cannon::KEntity_Projectile_Cannon() 
{
	GravityScale = 1;
	Bounciness = .4;
	PhysicsProperties.Deceleration = .875;
	PhysicsFluidProperties.Deceleration = .93;
	SinkSpeed = 350;
	Lifespan = 3;
	MaxStepHeight = 4;
	SetSkipGroundCheck(false);

	RotationRate = glm::vec3(RandFloat(-1, 1), RandFloat(-1, 1), RandFloat(-1, 1)) * 360.f;
	Rotation = glm::quat(RotationRate);
}

KEntity_Projectile_Cannon::~KEntity_Projectile_Cannon() {}

void KEntity_Projectile_Cannon::Poolable_OnInstantiated()
{

}

void KEntity_Projectile_Cannon::Poolable_PreDestroy()
{
	KEntity::Poolable_PreDestroy();

	if (KGameInstance::Get().IsDestroyingMatch()) return;

	if (KEntity_Weapon* wep = FiringWeapon.Get())
	if (KEntity_Weapon_Cannon* cannon = wep->As<KEntity_Weapon_Cannon>())
	for (u32 i = 0; i < cannon->Projectiles.size(); i++)
	{
		if (cannon->Projectiles[i].GetPoolIndex() == Poolable_GetIndex())
		{
			VectorRemoveAt(cannon->Projectiles, i);
			break;
		}
	}


	if (!IsMyNetProjectile())
	{
		if (!bExploded && HasObjectAuthority())
		{
			GHitResult hit;
			hit.Point = GetPosition();
			hit.Normal = { 0, 0, 1 };
			DisableCollision();
			SplashDamage(hit, Velocity, SplashRadius, 125);
			KEntity_Explosion::CreateExplosion(SplashRadius, hit.Point, GVec3(0, 0, 1));
		}

#if !_SERVER
		KSoundProperties props;
		props.Volume = 3;
		KSoundInstance boom = KAudio::PlaySound3D(KSoundID::Cannon_Explode_1, GetPosition(), props);
		KAudio::SetSoundSpeed(boom, RandRange(.9, 1.1));
#endif
	}
}

void KEntity_Projectile_Cannon::InitNetObject()
{
	KEntity_Projectile::InitNetObject();
	
	// server will update this
	/*DisableCollision();
	
	if (IsMyNetProjectile())
	{
		GFlt pitch, yaw;
		Velocity.ToPitchYaw(pitch, yaw);
		const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
		const GVec3 right = dir.Cross(GVec3(0, 0, 1));
		RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
	}*/
#if !_SERVER
	//else
	if (!IsMyNetProjectile())
	{
		EnableCollision();		
		BuildMatrix(PrevMatrix);
	}
#endif

	/*CollisionBlocks = ECollisionMask::WorldStatic;
	//CollisionOverlaps = ECollisionMask::Gib;
	CollisionOverlaps |= ECollisionMask::Water;
	//CollisionOverlaps |= ECollisionMask::Pickup;

	SetMovementEnabled(false);*/
}

TObjRef<KEntity_Projectile_Cannon> KEntity_Projectile_Cannon::Create(const KProjectileCreationParams& params)
{
	TObjRef<KEntity_Projectile_Cannon> projRef = TDataPool<KEntity_Projectile_Cannon>::GetPool()->CreateNew();
	KEntity_Projectile_Cannon* proj = projRef.Get();
	InitBaseProjectile(params, proj);

	proj->SetCollisionBoundsHalfExtent(6, proj->CompFrames > 0);

#if !_SERVER
	bool mine = true;
	if (GetLocalNetPlayer()) mine = GetLocalNetPlayer()->OwningPlayerIndex == params.OwningPlayerIndex;
	if (!mine) proj->NetType = ENetProjectileType::ServerSweepPre;
	proj->BuildMatrix(proj->PrevMatrix);
#else
	proj->NetType = ENetProjectileType::ServerSweepPre;
#endif

	return projRef;
}

void KEntity_Projectile_Cannon::BuildMatrix(glm::mat4& matrix)
{
	matrix = glm::mat4(1);

	matrix = glm::translate(matrix, (GetPosition() + RenderOffset).ToGLM());
	matrix = glm::scale(matrix, glm::vec3(6, 6, 6));

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

void KEntity_Projectile_Cannon::ProjectileTick()
{
#if !_SERVER
	//if (!HasObjectAuthority())
	//	SetPosition(GetNetPosition());
#endif

	if (KTime::FrameCount() == DestroyFrame)
		DestroyEntity();
}

void KEntity_Projectile_Cannon::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{
	if (IsMyNetProjectile()) return;
	if (hit.HitCollision & (ECollisionMask::MonsterCharacter | ECollisionMask::PlayerCharacter))
	{
		DisableCollision();

		SplashDamage(hit, preVel, SplashRadius, 125);
		bExploded = true;

		KEntity_Explosion::CreateExplosion(SplashRadius, hit.Point, GVec3(0, 0, 1));
		DestroyEntity();
	}
}

/*

void KEntity_Projectile_Cannon::OnMoveOverlap(const GVec3& preVel, const GHitResult& hit)
{
	if (PhysicsFloorNormal != 0)
	{
		if (hit.HitCollision & (ECollisionMask::PlayerCharacter | ECollisionMask::MonsterCharacter))
		{
			DestroyEntity();		
		}
	}
}
*/

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_Cannon::UpdateBuffers(KStaticMesh<"cannonball", "kf/stone1">& entry/*, KDynamicLight& light*/)
{
	// if this is my projectile spawned by the server i dont need to see it
	if (IsMyNetProjectile()) return false;

	entry.PrevModelMat = PrevMatrix;
	BuildMatrix(entry.CurrentModelMat);

	PrevMatrix = entry.CurrentModelMat;

	entry.SetLastMoveRenderAlpha(1);

	//entry.HalfExtent = glm::vec4(14, 24, 14, 1);

	//const GVec3 camPos = GetLocalPlayer()->CameraPosition;
	//const GFlt sqDist = camPos.DistanceSq(GetPosition());
	//const GFlt maxSqDist = 256 * 256;
	//if (sqDist < maxSqDist)
	{
		
		//GFlt distAlpha = 2 * (1 - (sqrt(sqDist) / sqrt(maxSqDist)));

		/*GFlt rad = 8;// * KSaturate(distAlpha);
		light.SetColor(FColor8(200, 200, 200, 255));
		light.SetFalloff(1);
		light.SetPrevRadius(rad);
		light.SetCurrentRadius(rad);
		light.SetNegative();
		light.SetLightAll();
		light.SetPrevPosition(LastFrameRenderPosition.AdjustZ(-6));
		light.SetCurrentPosition(GetPosition().AdjustZ(-6));*/
	}


	//FlagBufferForSkip(light);

	//return KBufferUpdateResult(false, false);
	return KBufferUpdateResult(LastFrameRenderPosition != GetPosition(), true);
}
#endif


void KEntity_Projectile_Cannon::OverlapCharacter(const GVec3& preVel, GHitResult& hit)
{
	SetPosition(hit.Point);

	GHitResult groundHit;
	InitHitResult(groundHit, true, false);
	TraceBox(GLineSegment(GetPosition(), GetPosition().AdjustZ(-4)), GetCollisionBoundsHalfExtent(), groundHit);

	if (groundHit.bHit && groundHit.Normal.z > .707)
		return;
	
	KEntity_Projectile::OverlapCharacter(preVel, hit);
}
