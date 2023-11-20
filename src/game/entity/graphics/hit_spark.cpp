#if !_SERVER
#include "hit_spark.h"
#include "engine/utility/random.h"
#include "engine/game_instance.h"

void KEntity_HitSpark::Create(const GVec3& pos, const GVec3& norm)
{
	auto spark = TDataPool<KEntity_HitSpark>::GetPool()->CreateNew().Get();
	spark->Position = pos.ToType<f32>();
	spark->Normal = norm.ToType<f32>();
}

KEntity_HitSpark::KEntity_HitSpark()
{
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
	RenderableLifespan = .5;
	RandomOffset = RandFloat(2, 10);
}

KBufferUpdateResult KEntity_HitSpark::UpdateBuffers(KHitSpark& entry)
{
	entry.SetNormal(Normal);
	entry.SetPosition(Position);
	entry.SetTimeCreated(RenderTimeCreated);
	entry.SetRandomOffset(RandomOffset);
	return KBufferUpdateResult(false, true); // never changes
	// TODO this hides the most recently spawned one on the frame a new one spawns
}

#endif