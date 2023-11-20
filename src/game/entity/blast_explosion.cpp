#include "blast_explosion.h"
#include "engine/game_instance.h"
#include "graphics/flash.h"

KEntity_BlastExplosion::KEntity_BlastExplosion()
{
#if !_SERVER
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
#endif
}

void KEntity_BlastExplosion::CreateExplosion(const GVec3& pos)
{
#if !_SERVER
	KEntity_BlastExplosion* explode = TDataPool<KEntity_BlastExplosion>::GetPool()->CreateNew().Get();

	explode->SetPosition(pos);

	auto flash = KEntity_LightFlash::CreateFlash(pos, FColor8(60, 60, 255, 255), 128, .05, .15, 4).Get();
	flash->PositionEntity = explode;

	KLocalPlayer* p = GetLocalPlayer();
	GFlt camDist = p->CameraPosition.DistanceSq(pos);
	camDist /= pow(128, 2);
	camDist = 1 - KSaturate(camDist);
	if (camDist > 0)
		p->ShakeCamera(camDist, .2 * camDist, 20);

	explode->RenderableLifespan = 1.f / 4.6f;
//#else
	// server only needs to do damage and destroy
	//explode->DestroyEntity();
#endif
}

#if !_SERVER
KBufferUpdateResult KEntity_BlastExplosion::UpdateBuffers(KBlastExplosion& entry)
{
	entry.SetPosition(GetPosition().ToType<f32>());
	entry.SetTimeCreated(RenderTimeCreated);
	return KBufferUpdateResult(false, true); // never changes
}
#endif