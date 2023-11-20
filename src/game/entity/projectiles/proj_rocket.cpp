#include "proj_rocket.h"
#include "engine/collision/trace.h"
#include "../weapon.h"
#include "../explosion.h"
#include "engine/utility/random.h"
#include "engine/audio/sound_asset.h"
#include "engine/audio/audio.h"
#include "engine/net/player.h"
#include "engine/game_instance.h"

KEntity_Projectile_Rocket::KEntity_Projectile_Rocket() {}
KEntity_Projectile_Rocket::~KEntity_Projectile_Rocket() {}

void KEntity_Projectile_Rocket::Poolable_OnInstantiated()
{
	
}

void KEntity_Projectile_Rocket::Poolable_PreDestroy()
{
	KEntity::Poolable_PreDestroy();

#if !_SERVER
	if (KEntity_RocketTrail* trail = Trail.Get())
	{
		trail->RenderableLifespan = 1;
		trail->RocketDeathTime = KGameInstance::Get().GetTotalRenderTime();
	}

	if (KAudio::SoundIsValid(LoopSound))
		KAudio::StopSound(LoopSound);

	if (KGameInstance::Get().IsDestroyingMatch()) return;

	if (!IsMyNetProjectile())
	{
		KSoundProperties props;
		props.Volume = 2.5;
		KSoundID sounds[] = 
		{
			KSoundID::Rocket_Explode_1,
			KSoundID::Rocket_Explode_2
		};
		KSoundInstance boom = KAudio::PlaySound3D(sounds[KTime::FrameCount() % 2], GetPosition(), props);
		KAudio::SetSoundSpeed(boom, RandRange(.9, 1.1));
	}
#endif
}

void KEntity_Projectile_Rocket::InitNetObject()
{
	KEntity_Projectile::InitNetObject();
#if !_SERVER
	if (!IsMyNetProjectile())
	{
		BuildMatrix(PrevMatrix);
		Trail = KEntity_RocketTrail::Create(GetPosition(), Velocity, false, false);
	}
#endif

/*
#if !_SERVER
	if (!IsMyNetProjectile())
	{
		GFlt pitch, yaw;
		Velocity.ToPitchYaw(pitch, yaw);
		const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
		const GVec3 right = dir.Cross(GVec3(0, 0, 1));
		RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
		Trail = KEntity_RocketTrail::Create(GetPosition(), Velocity, false, false);
		BuildMatrix(PrevMatrix);
	}
#endif

	CollisionBlocks = ECollisionMask::WorldStatic;
	CollisionOverlaps = ECollisionMask::Gib;
	CollisionOverlaps |= ECollisionMask::Water;
	CollisionOverlaps |= ECollisionMask::Pickup;*/

}

TObjRef<KEntity_Projectile_Rocket> KEntity_Projectile_Rocket::Create(const KProjectileCreationParams& params)
{
	TObjRef<KEntity_Projectile_Rocket> projRef = TDataPool<KEntity_Projectile_Rocket>::GetPool()->CreateNew();
	KEntity_Projectile_Rocket* proj = projRef.Get();
	InitBaseProjectile(params, proj);

#if !_SERVER
	bool mine = true;
	if (GetLocalNetPlayer()) mine = GetLocalNetPlayer()->OwningPlayerIndex == params.OwningPlayerIndex;
	if (!mine) proj->NetType = ENetProjectileType::ServerSweepPre;
	proj->Trail = KEntity_RocketTrail::Create(proj->GetPosition(), proj->Velocity, true, mine);
	proj->BuildMatrix(proj->PrevMatrix);
#else
	proj->NetType = ENetProjectileType::ServerSweepPre;
#endif

	return projRef;
}

void KEntity_Projectile_Rocket::OnNetDestroy()
{
	if (!IsMyNetProjectile())
		KEntity_Explosion::CreateExplosion(SplashRadius, DestroyPosition.ToVec3(), DestroyNormal.ToVec3());
}

void KEntity_Projectile_Rocket::BuildMatrix(glm::mat4& matrix)
{
	matrix = glm::mat4(1);

	matrix = glm::translate(matrix, (GetPosition() + RenderOffset).ToGLM());
	matrix = glm::scale(matrix, glm::vec3(1.5, 1.5, 1.5));

	// spinning
	GVec3 vel = LastFrameVelocity.GetNormalized();
	GVec3 cross = abs(LastFrameVelocity.z) >= .99 ? GVec3(0, 1, 0) : GVec3(0, 0, 1);
	GVec3 velRight = vel.Cross(cross).GetNormalized();
	GVec3 velUp = velRight.Cross(velRight).GetNormalized();
	velUp *= sin(GetEntityAge() * 8) * .05;
	velRight *= cos(GetEntityAge() * 8) * .05;
	vel += velUp + velRight;
	matrix = glm::rotate(matrix, (f32)GetEntityAge() * 20 + RandomByIndex(Poolable_GetID() % 256), vel.ToGLM());

	// face velocity
	GFlt p, y;
	LastFrameVelocity.ToPitchYaw(p, y);
	matrix *= glm::mat4_cast(glm::quat({ (f32)-p, (f32)y, 0 }));
}

void KEntity_Projectile_Rocket::ProjectileTick()
{
	if (!IsMyNetProjectile() && !KAudio::SoundIsValid(LoopSound))
	{
		KSoundProperties props;
		props.Volume = 0;
		props.bLooping = true;
		LoopSound = KAudio::PlaySound3D(KSoundID::Proj_Rocket_Hum, GetPosition(), props);
		KAudio::SetSoundSpeed(LoopSound, RandRange(.9, 1.1));
		KAudio::FadeVolume(LoopSound, .5, .5);
	}

	KAudio::UpdateSoundPosition(LoopSound, GetPosition());
}

void KEntity_Projectile_Rocket::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{
	DisableCollision();
	
	SplashDamage(hit, preVel, SplashRadius, MaxSplashDamage);

	if (IsNetServer())
	{
		DestroyPosition = hit.Point;
		DestroyNormal = hit.Normal;
	}

	KEntity_Explosion::CreateExplosion(SplashRadius, hit.Point, hit.Normal);
	DestroyEntity();
}

void KEntity_Projectile_Rocket::PreClientLocalTick(u32 frames)
{
#if !_SERVER
	if (frames != 1)
	{
		if (KEntity_RocketTrail* trail = Trail.Get())
		{
			i32 diff = -1 + frames;
			trail->TimeCreated + GameFrameDelta() * diff;
		}
	}
#endif
}

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_Rocket::UpdateBuffers(KStaticMesh<"rocket", "kf/colors/color_red">& entry, KDynamicLight& light)
{
	// if this is my projectile spawned by the server i dont need to see it
	if (IsMyNetProjectile()) return false;

	entry.PrevModelMat = PrevMatrix;
	BuildMatrix(entry.CurrentModelMat);

	PrevMatrix = entry.CurrentModelMat;

	entry.SetLastMoveRenderAlpha(1);

	//entry.HalfExtent = glm::vec4(14, 24, 14, 1);

	light.SetColor(FColor8(200, 150, 30, 255));
	light.SetFalloff(4);
	light.SetPrevRadius(96);
	light.SetCurrentRadius(96);
	light.SetPrevPosition(LastFrameRenderPosition);
	light.SetCurrentPosition(GetPosition());
	
	return KBufferUpdateResult(LastFrameRenderPosition != GetPosition(), true);
}
#endif

