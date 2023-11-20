#include "audio.h"
#include "../net/replicated_sound.h"
#include "../net/state.h"
#include "game/entity/entity.h"
#include "engine/net/snapshottable.h"

#if !_NOSOUND
#include "engine/game/config.h"
#include "soloud_wav.h"
#include "game/underwater_sound.h"
#include "../game_instance.h"
#include "sound_list.h"
#include "attached_sound.h"
#include "../input/view.h"

SoLoud::Soloud KAudio::Instance;
f32 KAudio::ConfiguredMasterVolume = -1;
f32 KAudio::MasterVolume = 1;
TVector<class KAttachedSound> KAudio::AttachedSounds;

KAudio::~KAudio() {}

void KAudio::Init()
{
	Instance.init();
	Instance.setMaxActiveVoiceCount(255);
	//Instance.createVoiceGroup(); // https://sol.gfxile.net/soloud/voicegroups.html
	SetMasterVolume(GetUserConfig()->Audio.MasterVolume);
	UpdateAudio();

	KSoundID::SoundList.resize(KSoundID::NumSounds);
	KSoundID::LoadSounds(); // load all sounds

	/*nice = new KSoundAsset;
	nice->LoadWave("thesound");
	KEntity_Character_Monster::ArnoldBitch = new KSoundAsset;
	KEntity_Character_Monster::ArnoldBitch->LoadWave("bitch");
	KBasicMenu::ScrollSound = new KSoundAsset;
	KBasicMenu::ScrollSound->LoadWave("switch");
	KBasicMenu::CloseSound = new KSoundAsset;
	KBasicMenu::CloseSound->LoadWave("menu_close");
	KBasicMenu::OpenSound = new KSoundAsset;
	KBasicMenu::OpenSound->LoadWave("menu_open");
	KMapEntity_Portal::AmbientSound = new KSoundAsset;
	KMapEntity_Portal::AmbientSound->LoadWave("portal_loop");
	KMapEntity_Portal::EnterSound = new KSoundAsset;
	KMapEntity_Portal::EnterSound->LoadWave("portal_enter");*/

	KUnderwaterSound::Init();

	LOG("Audio initialized");
}

void KAudio::Shutdown()
{
	Instance.deinit();
}

void KAudio::StopAllSounds()
{
	Instance.stopAll();
}

void KAudio::UpdateAudio()
{
	// volume
	f32 master = KGameInstance::Get().MasterVolume;
	if (master != ConfiguredMasterVolume)
	{
		ConfiguredMasterVolume = master;
		MasterVolume = pow(master, 2.7);
		Instance.setGlobalVolume(MasterVolume);
	}

	// 3d audio
	if (!KGameInstance::Get().IsPaused())
	{
		GVec3 dir = GVec3::FromPitchYaw(KInputView::GetPitch(), KInputView::GetYaw());
		//GVec3 right = dir.Cross(GVec3(0, 0, 1)).GetNormalized();

		for (i32 i = AttachedSounds.size() - 1; i >= 0; i--)
		{
			const KAttachedSound& sound = AttachedSounds[i];
			bool valid = false;
			if (SoundIsValid(sound.Instance))
			{
				if (KEntity* ent = sound.AttachedEntity.Get())
				{
					GVec3 position = ent->GetPosition();
					//if (GetDrawConfig()->Flip)
					//	position *= -right;

					UpdateSoundPosition(sound.Instance, position);
					valid = true;
				}
				else if (sound.bPlayAfterDeath)
					valid = true;
			}
			if (!valid) 
			{
				KAudio::StopSound(sound.Instance);
				VectorRemoveAt(AttachedSounds, i);
			}
		}

		GVec3 pos = GetLocalPlayer()->CameraPosition;
		Instance.set3dListenerPosition(pos.y, pos.z, pos.x);
		Instance.set3dListenerAt(dir.y, dir.z, dir.x);
		Instance.set3dListenerUp(0, 1, 0);
		Instance.update3dAudio();
		Instance.setPauseAll(false);
	}
}

void KAudio::UpdateSoundPosition(KSoundInstance inst, const GVec3& pos)
{
	Instance.set3dSourcePosition(inst.Handle, pos.y, pos.z, pos.x);	
}

void KAudio::SetMasterVolume(f32 volume)
{
	Instance.setGlobalVolume(volume);
}

bool KAudio::SoundIsValid(KSoundInstance inst)
{
	return Instance.isValidVoiceHandle(inst.Handle);
}

void KAudio::SetHandleVolume(KSoundInstance inst, f32 vol)
{
	Instance.setVolume(inst.Handle, vol);
}

void KAudio::StopSound(KSoundInstance inst)
{
	Instance.stop(inst.Handle);
}

void KAudio::SetSoundSpeed(KSoundInstance inst, f32 speed)
{
	Instance.setRelativePlaySpeed(inst.Handle, speed);
}

void KAudio::FadeVolume(KSoundInstance inst, f32 target, f64 time)
{
	Instance.fadeVolume(inst.Handle, target, time);
}

UPtr<KSoundAsset> KAudio::LoadSound(const KString& name)
{
	UPtr<KSoundAsset> sound = std::make_unique<KSoundAsset>();
	sound->LoadWave(name);
	return sound;
}

KSoundInstance KAudio::PlaySound(KSoundID soundID, f32 volume /*= -1*/, bool loop)
{
	KSoundInstance inst;
	inst.Handle = Instance.play(GetSoundAsset(soundID)->Wave, volume, 0, 0);
	Instance.setLooping(inst.Handle, loop);
	
	return inst;
}

KSoundInstance KAudio::PlaySound3D(KSoundID soundID, const GVec3& position, const KSoundProperties& props /*= KSoundProperties()*/)
{
	KSoundInstance inst = _PlaySoundAtPosition(soundID, position, props);

	if (IsNetServer())
	{
		if (props.ReplicationMethod != EReplicatedSound::LocalOnly)
		{
			KReppedSound repped;
			repped.SetPosition(position);
			repped.SetSoundID(soundID);
			GetNetState()->ProcessReppedSound(repped, props);
		}
	}

	return inst;
}

KSoundInstance KAudio::PlaySoundAttached(KSoundID id, class KEntity* ent, const KSoundProperties& props /*= KSoundProperties()*/)
{
	KSoundInstance inst;
	if (ent == GetViewedEntity())
		inst = _PlaySound2D(id, props);
	else
		inst = _PlaySoundAtPosition(id, ent->GetPosition(), props);

	if (SoundIsValid(inst))
	{
		KAttachedSound sound;
		sound.AttachedEntity = ent;
		sound.bPlayAfterDeath = props.bAttachedPlayAfterDeath;
		sound.Instance = inst;
		AttachedSounds.push_back(sound);
	}

	if (IsNetServer())
	{
		if (props.ReplicationMethod != EReplicatedSound::LocalOnly)
		{
			if (KSnapshottable* snap = ent->As<KSnapshottable>())
			{
				KReppedSound repped;
				repped.SetAttachNetID(snap->NetID, props.bAttachedPlayAfterDeath);
				repped.SetSoundID(id);
				GetNetState()->ProcessReppedSound(repped, props);
			}
		}
	}

	return inst;
}

KSoundInstance KAudio::_PlaySoundAtPosition(KSoundID soundID, const GVec3& pos, const KSoundProperties& props)
{
	KSoundInstance inst;
	inst.Handle = 0;
	if (!props.bRepOnly)
	{
		inst.Handle = Instance.play3d(
			GetSoundAsset(soundID)->Wave,
			pos.y,
			pos.z,
			pos.x,
			0, 0, 0,
			props.Volume, true);

		Instance.set3dSourceAttenuation(inst.Handle, SoLoud::AudioSource::KF_DISTANCE, .7);
		Instance.set3dSourceMinMaxDistance(inst.Handle, 5, props.MaxDistance);
		Instance.setLooping(inst.Handle, props.bLooping);
	}
	return inst;
}

KSoundInstance KAudio::_PlaySound2D(KSoundID soundID, const KSoundProperties& props)
{
	KSoundInstance inst;
	inst.Handle = Instance.play(GetSoundAsset(soundID)->Wave, props.Volume, 0, 0);
	Instance.setLooping(inst.Handle, props.bLooping);
	return inst;
}

#else

KSoundInstance KAudio::PlaySound3D(KSoundID soundID, const GVec3& position, const KSoundProperties& props /*= KSoundProperties()*/)
{
	if (IsNetServer())
	{
		if (props.ReplicationMethod != EReplicatedSound::LocalOnly)
		{
			KReppedSound repped;
			repped.SetPosition(position);
			repped.SetSoundID(soundID);
			GetNetState()->ProcessReppedSound(repped, props);
		}
	}
	return KSoundInstance();
}

KSoundInstance KAudio::PlaySoundAttached(KSoundID id, class KEntity* ent, const KSoundProperties& props /*= KSoundProperties()*/)
{
	if (IsNetServer())
	{
		if (props.ReplicationMethod != EReplicatedSound::LocalOnly)
		{
			if (KSnapshottable* snap = ent->As<KSnapshottable>())
			{
				KReppedSound repped;
				repped.SetAttachNetID(snap->NetID, props.bAttachedPlayAfterDeath);
				repped.SetSoundID(id);
				GetNetState()->ProcessReppedSound(repped, props);
			}
		}
	}
	return KSoundInstance();
}

#endif