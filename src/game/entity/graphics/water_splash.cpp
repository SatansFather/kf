#if !_SERVER

#include "water_splash.h"
#include "engine/utility/random.h"
#include "engine/game_instance.h"


void KEntity_WaterSplash::Create(const GVec3& pos, const GVec3& norm, FColor8 color, f32 scale, f32 strength)
{
	KEntity_WaterSplash* splash = TDataPool<KEntity_WaterSplash>::GetPool()->CreateNew().Get();
	splash->Position = pos.ToType<f32>();
	splash->Normal = norm.ToType<f32>();
	splash->Scale = scale;
	splash->Strength = strength;
	splash->Color = color;
}

KEntity_WaterSplash::KEntity_WaterSplash()
{
	RenderTimeCreated = KGameInstance::Get().GetTotalRenderTime();
	RenderableLifespan = .5;
	RandomOffset = RandFloat(2, 10);
}

KBufferUpdateResult KEntity_WaterSplash::UpdateBuffers(KWaterSplash& entry)
{
	entry.SetNormal(Normal);
	entry.SetPosition(Position);
	entry.SetTimeCreated(RenderTimeCreated);
	entry.SetColor(Color);
	entry.SetScale(Scale);
	entry.SetStrength(Strength);
	entry.SetRandomOffset(RandomOffset);
	return KBufferUpdateResult(false, true); // never changes
}

#endif