#include "explosion.h"
#include "graphics/flash.h"
#include "engine/game/local_player.h"
#include "engine/game_instance.h"
#include "graphics/rubble.h"

KEntity_Explosion::KEntity_Explosion()
{
#if !_SERVER
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
#endif
}

void KEntity_Explosion::CreateExplosion(f32 radius, const GVec3& pos, const GVec3& norm)
{
#if !_SERVER
	KEntity_Explosion* explode = TDataPool<KEntity_Explosion>::GetPool()->CreateNew().Get();

	explode->Radius = radius;
	explode->Normal = norm.ToType<f32>();
	explode->SetPosition(pos);

	auto flash = KEntity_LightFlash::CreateFlash(pos + norm * radius * .5, FColor8(255, 255, 150, 255), radius * 2, .05, .4, 4).Get();
	flash->PositionEntity = explode;

	KLocalPlayer* p = GetLocalPlayer();
	GFlt camDist = p->CameraPosition.DistanceSq(pos);
	camDist /= pow(radius * 4, 2);
	camDist = 1 - KSaturate(camDist);
	if (camDist > 0)
		p->ShakeCamera(2 * camDist, .3 * camDist, 40);

	explode->RenderableLifespan = 1.f / 1.8f;
	KEntity_Rubble::CreateExplosion(pos, norm);
//#else
	// server only needs to do damage and destroy
	//explode->DestroyEntity();
#endif
}

#if !_SERVER
KBufferUpdateResult KEntity_Explosion::UpdateBuffers(KExplosion& entry)
{
	entry.SetNormal(Normal);
	entry.SetPosition(GetPosition().ToType<f32>());
	entry.SetTimeCreated(RenderTimeCreated);
	entry.SetRadius(Radius);
	return KBufferUpdateResult(false, true); // never changes
}
#endif