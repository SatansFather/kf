#if !_SERVER

#include "portal_travel.h"
#include "engine/game_instance.h"

void KEntity_PortalTravel::Create(const GVec3& pos, const GVec3& halfExtent, bool isEntry)
{
	KEntity_PortalTravel* travel = TDataPool<KEntity_PortalTravel>::GetPool()->CreateNew().Get();

	travel->SetPosition(pos);
	travel->HalfExtent = halfExtent.ToType<f32>();
	travel->bIsEntry = isEntry;
}

KEntity_PortalTravel::KEntity_PortalTravel()
{
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
	RenderableLifespan = 3;
}

KBufferUpdateResult KEntity_PortalTravel::UpdateBuffers(KPortalTravel& entry)
{
	entry.SetPosition(GetPosition().ToType<f32>());
	entry.SetHalfExtent(HalfExtent.ToType<f32>());
	entry.SetTimeCreated(RenderTimeCreated);
	entry.SetIsEntry(bIsEntry);

	return true;
}

#endif