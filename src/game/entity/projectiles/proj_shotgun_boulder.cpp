#include "proj_shotgun_boulder.h"
#include "../character.h"
#include "../graphics/hit_spark.h"
#include "../../../engine/collision/trace.h"
#include "../graphics/bullet_hole.h"
#include "../../../engine/game/local_player.h"
#include "../../../engine/audio/sound_list.h"
#include "../../../engine/audio/audio.h"
#include "../../../engine/utility/random.h"
#include "../../../engine/net/player.h"
#include "proj_shotgun.h"

KEntity_Projectile_ShotgunBoulder::KEntity_Projectile_ShotgunBoulder()
{
	RotationRate = glm::vec3(RandFloat(-1, 1), RandFloat(-1, 1), RandFloat(-1, 1)) * 360.f;
	Rotation = glm::quat(RotationRate);
	DirectDamage = 70;
	GravityScale = 1;
}

void KEntity_Projectile_ShotgunBoulder::ProjectileTick()
{
#if !_SERVER
	//if (!HasObjectAuthority())
	//	SetPosition(GetNetPosition());
#endif
}

void KEntity_Projectile_ShotgunBoulder::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{
	if (hit.HitCollision >= ECollisionMask::PlayerCharacter) // bounding box entity
	{
		KEntProp_CollidableBBox* col = (KEntProp_CollidableBBox*)hit.Object;
		KEntProp_Killable* kill = dynamic_cast<KEntProp_Killable*>(col->GetEntity());
		KEntProp_Movable* move = dynamic_cast<KEntProp_Movable*>(col->GetEntity());

		if (move) move->Push(preVel * .045, 15);
		if (kill) kill->TakeDamage(DirectDamage, GetOwningPlayer());
	}

	// find average normal
	GVec3 avgNorm;
	u32 count = 0;
	for (i32 i = 0; i < 3; i++)
	for (i32 j = -1; j <= 1; j += 2)
	{
		GVec3 traceDir;
		traceDir[i] = j;

		GHitResult normHit;
		InitHitResult(normHit, true, false);
		TraceBox(GLineSegment(hit.Point, hit.Point + traceDir * 24), GetCollisionBoundsHalfExtent() * .6, normHit);
		if (normHit.bHit)
		{
			avgNorm += normHit.Normal;// * KSaturate((1 - normHit.Time) * 2);
			count++;
		}
	}

	if (count > 0)
	{
		avgNorm.Normalize();
	}
	else
	{
		avgNorm = GVec3(0, 0, 1);
	}

	GVec3 ref = preVel.Reflect(avgNorm);

	for (u32 i = 0; i < 16; i++)
	{
		KProjectileCreationParams params;
		params.Position = hit.Point;
		params.Direction = ref.GetNormalized();

		params.Direction.x += D_RandRange(-.45, .45);
		params.Direction.y += D_RandRange(-.45, .45);
		params.Direction.z += D_RandRange(-.45, .45);
		params.Direction.Normalize();

		params.Speed = 1200;
		params.FiringWeapon = FiringWeapon.Get();
		params.OwningPlayerIndex = OwningPlayerIndex;
		params.bPrimaryFire = false;
		KEntity_Projectile_ShotgunShard* proj = KEntity_Projectile_ShotgunShard::Create(params).Get();
		proj->IgnoreID = 0;
		proj->DirectDamage = 20;
		proj->FrameTickInterval = 2;
	}

#if !_SERVER
	KEntity_HitSpark::Create(hit.Point - hit.Normal * 4, hit.Normal);
#endif

	DestroyEntity();
}

void KEntity_Projectile_ShotgunBoulder::InitNetObject()
{
#if !_SERVER
	KEntity_Projectile::InitNetObject();
	LastFrameRenderPosition = GetPosition();
#endif
	/*DisableCollision();

	if (!IsMyNetProjectile())
	{
		GFlt pitch, yaw;
		Velocity.ToPitchYaw(pitch, yaw);
		const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
		const GVec3 right = dir.Cross(GVec3(0, 0, 1));
		RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
	}

	const GVec3 dir = Velocity.GetNormalized();

	CollisionBlocks = ECollisionMask::WorldStatic;
	//CollisionOverlaps = ECollisionMask::Gib;
	CollisionOverlaps |= ECollisionMask::Water;
	//CollisionOverlaps |= ECollisionMask::Pickup;

	SetMovementEnabled(false);*/
}

TObjRef<KEntity_Projectile_ShotgunBoulder> KEntity_Projectile_ShotgunBoulder::Create(const KProjectileCreationParams& params)
{
	TObjRef<KEntity_Projectile_ShotgunBoulder> projRef = TDataPool<KEntity_Projectile_ShotgunBoulder>::GetPool()->CreateNew();
	KEntity_Projectile_ShotgunBoulder* proj = projRef.Get();
	InitBaseProjectile(params, proj);

	proj->SetCollisionBoundsHalfExtent(8, proj->CompFrames > 0);

	return projRef;
}

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_ShotgunBoulder::UpdateBuffers(KStaticMesh<"meat1", "kf/stone2">& entry)
{
	if (IsMyNetProjectile()) return false;

	entry.PrevModelMat = PrevMatrix;
	BuildMatrix(entry.CurrentModelMat);

	PrevMatrix = entry.CurrentModelMat;

	entry.SetLastMoveRenderAlpha(1);

	return KBufferUpdateResult(LastFrameRenderPosition != GetPosition(), true);
}
#endif

static u32 LastHitSoundFrame = 0;

void KEntity_Projectile_ShotgunBoulder::BuildMatrix(glm::mat4& matrix)
{
	matrix = glm::mat4(1);

	matrix = glm::translate(matrix, (GetPosition() + RenderOffset).ToGLM());
	matrix = glm::scale(matrix, glm::vec3(13, 13, 13));

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

void KEntity_Projectile_ShotgunBoulder::Poolable_PreDestroy()
{
	KEntity::Poolable_PreDestroy();

#if !_SERVER
	/*if (!ClientData.bIsMine)
	{
		if (LastHitSoundFrame != KTime::FrameCount())
		{
			
			KSoundProperties props;
			KAudio::PlaySound3D(sounds[Random() % 4], GetPosition(), props);
			LastHitSoundFrame = KTime::FrameCount();
		}
	}*/
#endif
}

