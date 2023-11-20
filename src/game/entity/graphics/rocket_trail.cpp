#if !_SERVER
#include "rocket_trail.h"
#include "engine/game_instance.h"

KEntity_RocketTrail::KEntity_RocketTrail() {}

TObjRef<KEntity_RocketTrail> KEntity_RocketTrail::Create(const GVec3& startPos, const GVec3& vel, bool offset, bool mine)
{
	auto trail = TDataPool<KEntity_RocketTrail>::GetPool()->CreateNew();

	KEntity_RocketTrail* t = trail.Get();
	t->StartPosition = startPos.ToType<f32>();
	t->Velocity = vel.ToType<f32>();
	t->TimeCreated = KGameInstance::Get().GetTotalRenderTime();
	t->bUseRenderOffset = offset;
	t->bIsMyRocket = mine;

	return trail;
}

KBufferUpdateResult KEntity_RocketTrail::UpdateBuffers(KRocketTrail& entry)
{
	entry.SetStartPosition(StartPosition);
	entry.SetVelocity(Velocity);
	entry.SetTimeCreated(TimeCreated);
	entry.SetDeathTime(RocketDeathTime);
	if (bUseRenderOffset) entry.EnableRenderOffset();
	if (bIsMyRocket) entry.EnableMyRocket();

	KBufferUpdateResult result(RocketDeathTime != LastRocketDeathTime, true);
	LastRocketDeathTime = RocketDeathTime;

	return result;
}

#endif
