#include "proj_atom.h"
#include "engine/game/local_player.h"

KEntity_Projectile_Atom::KEntity_Projectile_Atom()
{
	SetCollisionBoundsHalfExtent(GVec3(16, 16, 16));
}

void KEntity_Projectile_Atom::ProjectileTick()
{
	Velocity *= .98;
	if (Velocity.LengthSq() < 5 && StopTime == -1) 
	{
		Velocity = 0;
		StopTime = KTime::FrameNow();
	}

#if !_SERVER

	GVec3 cam = GetLocalPlayer()->CameraPosition;
	GFlt dist = cam.Distance(GetPosition());
	GetLocalPlayer()->ShakeCamera(1 - KSaturate(dist / 512.f), .1, 16);

#endif
}

void KEntity_Projectile_Atom::OnProjectileHit(const GVec3& preVel, const GHitResult& hit)
{

}

#if !_SERVER
KBufferUpdateResult KEntity_Projectile_Atom::UpdateBuffers(KAtomProjectile& entry, KDynamicLight& light)
{
	entry.SetCurrentPosition(GetPosition());
	entry.SetPrevPosition(LastFrameRenderPosition);
	entry.SetLastMoveAlpha(GetLastMoveAlpha());

	light.SetPrevPosition(LastFrameRenderPosition);
	light.SetCurrentPosition(GetPosition());
	light.SetColor(FColor8(150, 200, 200, 0));
	light.SetFalloff(1);
	if (StopTime > 0)
	{
		
		light.SetPrevRadius((KTime::FrameNow() - StopTime) * 192);
		light.SetCurrentRadius((KTime::FrameNow() - StopTime) * 192);
	}

	return true;
}
#endif