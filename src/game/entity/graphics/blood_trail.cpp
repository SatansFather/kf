#if !_SERVER

#include "blood_trail.h"
#include "engine/utility/random.h"
#include "engine/game_instance.h"

void KEntity_BloodTrail::Create(const GVec3& position)
{
	auto blood = TDataPool<KEntity_BloodTrail>::GetPool()->CreateNew().Get();
	blood->SetPosition(position);
}

KEntity_BloodTrail::KEntity_BloodTrail()
{
	TimeCreated = KGameInstance::Get().GetTotalRenderTime();
	RenderableLifespan = .7;
}

KBufferUpdateResult KEntity_BloodTrail::UpdateBuffers(KBloodTrail& entry)
{
	entry.SetPosition(GetPosition());
	entry.SetTimeCreated(TimeCreated);
	return KBufferUpdateResult(false, true);
}

void KEntity_BloodTrail_UnderWater::Create(const GVec3& position)
{
	auto blood = TDataPool<KEntity_BloodTrail_UnderWater>::GetPool()->CreateNew().Get();
	blood->SetPosition(position);
}

KEntity_BloodTrail_UnderWater::KEntity_BloodTrail_UnderWater()
{
	TimeCreated = KGameInstance::Get().GetTotalRenderTime();
	RenderableLifespan = .7;
}

KBufferUpdateResult KEntity_BloodTrail_UnderWater::UpdateBuffers(KBloodTrail_UnderWater& entry)
{
	entry.SetPosition(GetPosition());
	entry.SetTimeCreated(TimeCreated);
	return KBufferUpdateResult(false, true);
}

#endif