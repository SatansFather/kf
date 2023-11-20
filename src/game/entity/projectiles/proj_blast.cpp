#include "proj_blast.h"
#include "engine/game_instance.h"
#include "../blast_explosion.h"



KEntity_Projectile_Blast::KEntity_Projectile_Blast()
{
	SetCollisionBoundsHalfExtent(8, true);
	PushScale = 150;
#if !_SERVER
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
#endif
}

KEntity_Projectile_Blast::~KEntity_Projectile_Blast()
{

}

void KEntity_Projectile_Blast::InitNetObject()
{
	KEntity_Projectile::InitNetObject();
#if !_SERVER
	LastFrameRenderPosition = GetPosition();
#endif
}

TObjRef<KEntity_Projectile_Blast> KEntity_Projectile_Blast::Create(const KProjectileCreationParams& params)
{
	TObjRef<KEntity_Projectile_Blast> projRef = TDataPool<KEntity_Projectile_Blast>::GetPool()->CreateNew();
	KEntity_Projectile_Blast* proj = projRef.Get();
	InitBaseProjectile(params, proj);
	//proj->DirectDamage = 35;
	
	return projRef;
}

void KEntity_Projectile_Blast::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{
	/*if (hit.HitCollision >= ECollisionMask::PlayerCharacter) // bounding box entity
	{
		KEntProp_CollidableBBox* col = (KEntProp_CollidableBBox*)hit.Object;
		KEntProp_Killable* kill = dynamic_cast<KEntProp_Killable*>(col->GetEntity());
		KEntProp_Movable* move = dynamic_cast<KEntProp_Movable*>(col->GetEntity());

		if (move) move->Push(preVel * .05, 15);
		if (kill) kill->TakeDamage(DirectDamage, GetOwningPlayer());
	}*/

	DisableCollision();

	SplashDamage(hit, preVel, 64, 35, 0, 0);

	if (IsNetServer()) DestroyPosition = hit.Point;
	KEntity_BlastExplosion::CreateExplosion(hit.Point);
	DestroyEntity();
}

void KEntity_Projectile_Blast::OnNetDestroy()
{
	if (!IsMyNetProjectile())
		KEntity_BlastExplosion::CreateExplosion(DestroyPosition.ToVec3());
}

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_Blast::UpdateBuffers(KBlasterParticle& entry, KDynamicLight& light)
{
	if (IsMyNetProjectile()) return false;

	light.SetColor(FColor8(60, 60, 255, 255));
	light.SetCurrentPosition(GetPosition());
	light.SetPrevPosition(LastFrameRenderPosition);
	light.SetPrevRadius(64);
	light.SetCurrentRadius(64);
	light.SetFalloff(4);
	FlagBufferForSkip(light);

	entry.SetPrevPosition(LastFrameRenderPosition + LastRenderOffset);
	entry.SetCurrentPosition(GetPosition() + RenderOffset);
	entry.SetTimeCreated(RenderTimeCreated);

	return true;
}
#endif