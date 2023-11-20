#if !_SERVER

#include "shotgun_trail.h"
#include "engine/utility/random.h"

KEntity_ShotgunTrail::KEntity_ShotgunTrail()
{
	RandomTimeOffset = RandFloat(0, 2);
	RenderableLifespan = 1;
	Scale = 1;
}

void KEntity_ShotgunTrail::Tick()
{
	//if (GetEntityAge() >= Lifetime) DestroyEntity();
}

KBufferUpdateResult KEntity_ShotgunTrail::UpdateBuffers(KSmokeBeam& entry)
{
	KRenderBufferInfo& info = GetInfoFromGame(&typeid(KSmokeBeam));
	if (info.LastFrameUpdated != KTime::FrameCount()) 
	{
		KSmokeBeam::CpuParticleCount = 0;
		info.ActiveCount = 0;
	}
	Color = FColor8(200, 160, 150, 25);
	//Color = FColor8(200, 200, 200, 100);

	u32 preCount = KSmokeBeam::CpuParticleCount;
	u32 diff = u32((EndPos - StartPos).Length()) / u32(10);

	if (KSmokeBeam::CpuParticleCount + diff > SMOKE_BEAM_MAX_PARTICLES) return false;

	KSmokeBeam::CpuParticleCount += diff;


	//u32 diff = KSmokeBeam::CpuParticleCount - preCount;
	std::fill_n(KSmokeBeam::CpuIndexMap.begin() + preCount, diff, info.ActiveCount);
	
	entry.SetStartPos(StartPos);
	entry.SetEndPos(EndPos);
	entry.SetLastEndPos(LastEndPos);
	entry.SetRadius(Scale);
	entry.SetSpacing(10);
	entry.SetColor(Color);
	entry.SetTimeOffset(RandomTimeOffset);
	entry.SetAge(GetEntityAge());
	entry.SetStartIndex(preCount);

	KBufferUpdateResult res(/*LastEndPos != EndPos*/ true, true);
	LastEndPos = EndPos;
	return res;
}

#endif