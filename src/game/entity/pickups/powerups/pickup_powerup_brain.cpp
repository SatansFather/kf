#include "pickup_powerup_brain.h"
#include "engine/render/interface/render_interface.h"
#include "../../powerup_index.h"
#include "../../powerups/powerup_brain.h"
#include "../../../../engine/audio/audio.h"

KEntity_Pickup_Powerup_Brain::KEntity_Pickup_Powerup_Brain()
{
	PowerupIndex = EPowerupID::Brain;
	SetStartSpawned(true);
	RespawnFrameCount = KTime::FramesFromTime(60);
}

TObjRef<KEntity_Powerup> KEntity_Pickup_Powerup_Brain::CreatePowerup()
{
	return TDataPool<KEntity_Powerup_Brain>::GetPool()->CreateNew();
}

#if !_SERVER
KBufferUpdateResult KEntity_Pickup_Powerup_Brain::UpdateBuffers(KPowerupBrain& entry, KDynamicLight& light)
{
	glm::mat4 mat(1);

	TotalTime += GameFrameDelta() / 2;

	bool spawned = IsSpawnedForViewedEntity() || IsDropItem();

	f32 changeFrames = KTime::FramesSince(SpawnStateChangeFrame);
	f32 changeAlpha = KSaturate(changeFrames / 30.f);
	if (changeAlpha < 1) changeAlpha = LerpFade(changeAlpha);

	f32 z = IsDropItem() ? 0 : (10 + sin( -( PI<f32>() / 4.f ) + changeFrames * GameFrameDelta() * 2) * 4);

	if (IsDropItem()) changeAlpha = 1;

	if (spawned)
	{
		f32 rotAlpha = KSaturate(changeFrames / 70);
		if (rotAlpha < 1) rotAlpha = LerpFade(rotAlpha);

		f32 scaleAlpha = KSaturate(changeFrames / 50);
		if (scaleAlpha < 1) scaleAlpha = LerpFade(scaleAlpha);

		if (IsDropItem())
		{
			scaleAlpha = KSaturate(f32(FrameDuration) / 30.f);
			rotAlpha = 1;
		}

		TotalTime += (1 - rotAlpha) * GameFrameDelta() * 22;

		entry.PrevModelMat = glm::translate(mat, LastFrameRenderPosition.AdjustZ(LastZ).ToGLM());
		entry.PrevModelMat = glm::scale(entry.PrevModelMat, glm::vec3(20, 16, 20) * LastScaleAlpha);
		entry.PrevModelMat = glm::rotate(entry.PrevModelMat, (IsDropItem() ? GetFrameCreated() : LastRenderTime), glm::vec3(0, 1, 0));;
		entry.CurrentModelMat = glm::translate(mat, GetPosition().AdjustZ(z).ToGLM());
		entry.CurrentModelMat = glm::scale(entry.CurrentModelMat, glm::vec3(20, 16, 20) * scaleAlpha);
		entry.CurrentModelMat = glm::rotate(entry.CurrentModelMat, (IsDropItem() ? GetFrameCreated() : TotalTime), glm::vec3(0, 1, 0));;
		entry.SetLastMoveRenderAlpha(GetLastMoveAlpha());

		LastScaleAlpha = scaleAlpha;
	}
	else
	{
		FlagBufferForSkip(entry);
		LastScaleAlpha = 0;
	}

	light.SetPrevPosition(LastFrameRenderPosition.AdjustZ(LastZ));
	light.SetCurrentPosition(GetPosition().AdjustZ(z));
	light.SetFalloff(1);
	light.SetColor(FColor8(255, 100, 255, 255));

	f32 fromRadius = spawned ? 64 : 192;
	f32 toRadius = spawned ? 192 : 64;
	f32 radius = KLerp(fromRadius, toRadius, changeAlpha);

	light.SetPrevRadius(LastRadius);
	light.SetCurrentRadius(radius);

	LastRenderTime = TotalTime;
	LastZ = z;
	LastRadius = radius;

	return true;
}
#endif

void KEntity_Pickup_Powerup_Brain::Tick()
{
#if !_DEV
	if (AmbientSound.GetHandle() == 0)
	{
		KSoundProperties props;
		props.bLooping = true;
		AmbientSound = KAudio::PlaySoundAttached(KSoundID::Ambient_Brain, this, props);
	}
	KAudio::SetHandleVolume(AmbientSound, IsSpawned() ? .5 : 0);
#endif

	KEntity_Pickup_Powerup::Tick();
}
