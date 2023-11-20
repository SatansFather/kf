#if !_NOSOUND
#include "underwater_sound.h"
#include "engine/audio/sound_asset.h"
#include "engine/game/local_player.h"
#include "../engine/collision/trace.h"

KSoundInstance KUnderwaterSound::DeepInstance;
KSoundInstance KUnderwaterSound::ShallowInstance;
bool KUnderwaterSound::bUnderwaterLastFrame = false;

KUnderwaterSound::~KUnderwaterSound() {}

void KUnderwaterSound::Init()
{
	DeepInstance = KAudio::PlaySound(KSoundID::Ambient_Underwater_Deep, 0, true);
	ShallowInstance = KAudio::PlaySound(KSoundID::Ambient_Underwater_Shallow, 0, true);
}

void KUnderwaterSound::Update()
{
	f32 depth = GetLocalPlayer()->WaterDepth;
	FColor32 color;
	if (depth > 0 && PointIsInWater(GetLocalPlayer()->CameraPosition, color))
	{
		bUnderwaterLastFrame = true;
		const f32 shallow = 64;
		const f32 deep = 192;
		const f32 alpha = KSaturate(MapRange(depth, shallow, deep, 0, 1));
		if (alpha <= 1)
		{
			f32 volume = 1 - alpha;
			volume *= KSaturate(MapRange(depth, 0, 128, 0, 1));
			volume *= .5;

			if (KAudio::SoundIsValid(ShallowInstance))
				KAudio::SetHandleVolume(ShallowInstance, volume);
			else
				ShallowInstance = KAudio::PlaySound(KSoundID::Ambient_Underwater_Shallow, volume, true);
		}
		else
		{
			KAudio::StopSound(ShallowInstance);
		}

		if (alpha >= 0)
		{
			f32 volume = alpha / 2;
			
			if (KAudio::SoundIsValid(DeepInstance))
				KAudio::SetHandleVolume(DeepInstance, volume);
			else
				DeepInstance = KAudio::PlaySound(KSoundID::Ambient_Underwater_Deep, volume, true);
		}
		else
		{
			KAudio::StopSound(DeepInstance);
		}
	}
	else if (bUnderwaterLastFrame)
	{
		bUnderwaterLastFrame = false;
		KAudio::FadeVolume(DeepInstance, 0, .1);
		KAudio::FadeVolume(ShallowInstance, 0, .1);
	}
}
#endif