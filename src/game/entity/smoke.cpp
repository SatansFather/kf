/*
#if !_SERVER

#include "smoke.h"
#include "engine/utility/random.h"

KEntity_DynamicSmoke::KEntity_DynamicSmoke()
{
	RandomTimeOffset = RandFloat(0, 2);
}

void KEntity_DynamicSmoke::Tick()
{
	if (GetEntityAge() >= Lifetime) DestroyEntity();
}

KBufferUpdateResult KEntity_DynamicSmoke::UpdateBuffers(KDynamicSmokeParticle& entry)
{
	glm::vec3 pos = LastPos.ToGLM();
	entry.PrevPos.x = pos.x;
	entry.PrevPos.y = pos.y;
	entry.PrevPos.z = pos.z;

	pos = GetPosition().ToGLM();
	entry.CurrentPos.x = pos.x;
	entry.CurrentPos.y = pos.y;
	entry.CurrentPos.z = pos.z;

	entry.PrevPos.w = LastScale;
	entry.CurrentPos.w = Scale;// + GetEntityAge() * 16;

	f32 rg = .5;
	//if (GetEntityAge() > 2) rg += (GetEntityAge() - 2);

	entry.SetColor ( { R, G, B, A } );
	entry.SetTimeOffset(RandomTimeOffset);

	LastPos = GetPosition();
	LastScale = entry.CurrentPos.w;

	return true;
}

#endif*/