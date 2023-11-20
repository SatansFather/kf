#if !_SERVER

#include "flash.h"
#include "engine/game_instance.h"

// DELETE
#include "../character_player.h"
#include "../projectile.h"

TObjRef<KEntity_LightFlash> KEntity_LightFlash::CreateFlash(const GVec3& position, FColor8 color, f32 radius, f32 growTime, f32 dimTime, f32 falloff)
{
	auto light = TDataPool<KEntity_LightFlash>::GetPool()->CreateNew();
	if (KEntity_LightFlash* l = light.Get()) 
	{
		l->SetPosition(position);
		l->RenderableLifespan = growTime + dimTime;
		l->FlashData.SetCurrentPosition(position);
		l->FlashData.SetPrevPosition(position);
		l->FlashData.SetColor(color);
		l->FlashData.SetRadius(radius);
		l->FlashData.SetGrowDuration(growTime);
		l->FlashData.SetDimDuration(dimTime);
		l->FlashData.SetTimeCreated(KGameInstance::Get().GetTotalRenderTime());
		l->FlashData.SetFalloff(falloff);
		l->RenderableLifespan = growTime + dimTime;
	}
	return light;
}

KBufferUpdateResult KEntity_LightFlash::UpdateBuffers(KLightFlash& entry)
{
	entry = FlashData;
	if (PositionEntity.IsValid())
	{
		if (KEntity_Character_Player* guy = PositionEntity.As<KEntity_Character_Player>())
		{			
			entry.SetPrevPosition(guy->LastFrameRenderPosition);
			entry.SetCurrentPosition(guy->GetPosition());
		}
	}

	return KBufferUpdateResult(true, true);
}

#endif
