#include "proj_shotgun.h"
#include "../character.h"
#include "../graphics/hit_spark.h"
#include "../../../engine/collision/trace.h"
#include "../graphics/bullet_hole.h"
#include "engine/game/local_player.h"
#include "engine/audio/sound_list.h"
#include "engine/audio/audio.h"
#include "engine/utility/random.h"
#include "engine/net/player.h"


void KEntity_Projectile_ShotgunShard::ProjectileTick()
{
#if !_SERVER
	if (SmokeBeam.IsValid())
	{
		//SmokeBeam->LastEndPos = SmokeBeam->EndPos;
		SmokeBeam.Get()->EndPos = GetPosition() + RenderOffset;
	}
#endif
}

void KEntity_Projectile_ShotgunShard::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{
#if !_SERVER
	if (SmokeBeam.IsValid()) SmokeBeam.Get()->EndPos = hit.Point + RenderOffset;

	GHitResult holeHit;
	holeHit.SearchCollision = ECollisionMask::WorldStatic;
	TraceLine(GLineSegment(GetPosition(), GetPosition() - hit.Normal * 8), holeHit);
	if (holeHit) KEntity_BulletHole::Create(holeHit.Point, holeHit.Normal);
#endif // server

	if (hit.HitCollision >= ECollisionMask::PlayerCharacter) // bounding box entity
	{
		KEntProp_CollidableBBox* col = (KEntProp_CollidableBBox*)hit.Object;
		KEntProp_Killable* kill = dynamic_cast<KEntProp_Killable*>(col->GetEntity());
		KEntProp_Movable* move = dynamic_cast<KEntProp_Movable*>(col->GetEntity());
		
		if (move && GravityScale == 0) move->Push(preVel * .02, 15);
		if (kill) kill->TakeDamage(DirectDamage, GetOwningPlayer());

		if (bPrimaryFire)
		{
			DestroyEntity();
			return;
		}
	}

	if (!bPrimaryFire)
	{
		if (BouncesRemaining > 0)
		{
			u32 frame = KTime::FrameCount();
			if (frame != LastBounceFrame)
			{
				BouncesRemaining--;
				LastBounceFrame = frame;
				DirectDamage -= 2;
			}

			return;
		}
	}

#if !_SERVER
	KEntity_HitSpark::Create(hit.Point - hit.Normal * 4, hit.Normal);
#endif
	DestroyEntity();
}

void KEntity_Projectile_ShotgunShard::InitNetObject()
{
#if !_SERVER
	KEntity_Projectile::InitNetObject();
	LastFrameRenderPosition = GetPosition();
	/*if (!IsMyNetProjectile())
	{
		GFlt pitch, yaw;
		Velocity.ToPitchYaw(pitch, yaw);
		const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
		const GVec3 right = dir.Cross(GVec3(0, 0, 1));
		RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
	}*/

	if (!IsMyNetProjectile())
	{
		if (bPrimaryFire)
		{
			const GVec3 dir = Velocity.GetNormalized();
			SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
			KEntity_ShotgunTrail* beam = SmokeBeam.Get();
			beam->StartPos = GetPosition() + RenderOffset + (dir * 24);
			beam->LastEndPos = beam->StartPos;
		}
		else
		{
			CollisionBlocks = ECollisionMask::WorldStatic;
			CollisionOverlaps |= ECollisionMask::Water;
			EnableCollision();
			GravityScale = 1;
		}
	}
#endif
}

TObjRef<KEntity_Projectile_ShotgunShard> KEntity_Projectile_ShotgunShard::Create(const KProjectileCreationParams& params)
{
	TObjRef<KEntity_Projectile_ShotgunShard> projRef = TDataPool<KEntity_Projectile_ShotgunShard>::GetPool()->CreateNew();
	KEntity_Projectile_ShotgunShard* proj = projRef.Get();
	InitBaseProjectile(params, proj);
	proj->DirectDamage = 20;
	if (params.bPrimaryFire)
	{
		proj->Lifespan = 1;
	}
	else
	{
		proj->Lifespan = 10;
		proj->GravityScale = 1;
	}

#if !_SERVER
	if (params.bPrimaryFire)
	{
		proj->SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
		KEntity_ShotgunTrail* beam = proj->SmokeBeam.Get();
		beam->StartPos = proj->GetPosition() + proj->RenderOffset + (params.Direction * 24);
		beam->LastEndPos = beam->StartPos;
	}
#endif

	return projRef;
}

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_ShotgunShard::UpdateBuffers(KShotgunShard& entry)
{
	if (IsMyNetProjectile()) return false;
	
	const GVec3 pre = FrameTickInterval == 0 ? LastFrameRenderPosition :
		GVec3::Lerp(LastFrameRenderPosition, GetPosition(), f32(FramesSinceMove) / f32(FrameTickInterval));
	const GVec3 post = FrameTickInterval == 0 ? GetPosition() : 
		GVec3::Lerp(LastFrameRenderPosition, GetPosition(), f32(FramesSinceMove + 1) / f32(FrameTickInterval));

	entry.SetPrevPosition(pre + LastRenderOffset);
	entry.SetCurrentPosition(post + RenderOffset);
	entry.SetLastMoveAlpha(GetLastMoveAlpha());
	entry.SetRandomSeed(RandomSeed);

	//entry.HalfExtent = glm::vec4(14, 24, 14, 1);

	//return KBufferUpdateResult(false, false);
	return KBufferUpdateResult(LastFrameRenderPosition != GetPosition(), true);
}
#endif

static u32 LastHitSoundFrame = 0;

void KEntity_Projectile_ShotgunShard::OnEntityDestroyed()
{
#if !_SERVER
	if (!IsMyNetProjectile())
	{
		if (LastHitSoundFrame != KTime::FrameCount())
		{
			KSoundID sounds[] = 
			{
				KSoundID::Shard_Hit_1,
				KSoundID::Shard_Hit_2,
				KSoundID::Shard_Hit_3,
				KSoundID::Shard_Hit_4,
			};
			KSoundInstance inst = KAudio::PlaySound3D(sounds[Random() % 4], GetPosition());
			LastHitSoundFrame = KTime::FrameCount();
			KAudio::SetSoundSpeed(inst, 2);
		}
	}
#endif
}
