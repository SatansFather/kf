#pragma once

#include "kfglobal.h"
#include "engine/math/vec3.h"
#include "sound_instance.h"
#include "sound_list.h"

enum EReplicatedSound
{
	LocalOnly,
	SendAll,
	SkipIndex,
	SendIndex
};

struct KSoundProperties
{
	f32 Volume = 1;
	f32 MaxDistance = 3064;
	bool bLooping = false;
	EReplicatedSound ReplicationMethod = EReplicatedSound::LocalOnly;
	u8 PlayerIndex = NULL_PLAYER;
	bool bAttachedPlayAfterDeath = false;

	// server wont play it
	bool bRepOnly = false;

	// server wont hold onto this sound for too long regardless of clients hearing it or not
	bool bLowHoldFrames = false;

	KSoundProperties() = default;
};

#if !_NOSOUND
#include "soloud.h"

class KAudio
{
	static SoLoud::Soloud Instance;
	static f32 ConfiguredMasterVolume;
	static f32 MasterVolume;

	static TVector<class KAttachedSound> AttachedSounds;

public:

	~KAudio();

	static void Init();
	static void Shutdown();
	static void StopAllSounds();
	static void UpdateAudio();
	static void UpdateSoundPosition(KSoundInstance inst, const GVec3& pos);
	static void SetMasterVolume(f32 volume);
	static bool SoundIsValid(KSoundInstance inst);
	static void SetHandleVolume(KSoundInstance inst, f32 vol);
	static void StopSound(KSoundInstance inst);
	static void SetSoundSpeed(KSoundInstance inst, f32 speed);
	static void FadeVolume(KSoundInstance inst, f32 target, f64 time);
	static UPtr<class KSoundAsset> LoadSound(const KString& name);

	static KSoundInstance PlaySound(KSoundID soundID, f32 volume = -1, bool loop = false);
	static KSoundInstance PlaySound3D(KSoundID soundID, const GVec3& position, const KSoundProperties& props = KSoundProperties());
	static KSoundInstance PlaySoundAttached(KSoundID id, class KEntity* ent, const KSoundProperties& props = KSoundProperties());

private:

	static KSoundInstance _PlaySoundAtPosition(KSoundID soundID, const GVec3& pos, const KSoundProperties& props);
	static KSoundInstance _PlaySound2D(KSoundID soundID, const KSoundProperties& props);

};

#else

// empty sound interface so audio calls compile

class KAudio
{

public:

	~KAudio() {}

	static void Init() {}
	static void Shutdown() {}
	static void StopAllSounds() {}
	static void UpdateAudio() {}
	static void UpdateSoundPosition(KSoundInstance inst, const GVec3& pos) {}
	static void SetMasterVolume(f32 volume) {}
	static bool SoundIsValid(KSoundInstance inst) { return false; }
	static void SetHandleVolume(KSoundInstance inst, f32 vol) {}
	static void StopSound(KSoundInstance inst) {}
	static void SetSoundSpeed(KSoundInstance inst, f32 speed) {}
	static void FadeVolume(KSoundInstance inst, f32 target, f64 time) {}
	static UPtr<class KSoundAsset> LoadSound(const KString& name) { return nullptr; }

	static KSoundInstance PlaySound(KSoundID soundID, f32 volume = -1, bool loop = false) { return KSoundInstance(); }
	static KSoundInstance PlaySound3D(KSoundID soundID, const GVec3& position, const KSoundProperties& props = KSoundProperties());
	static KSoundInstance PlaySoundAttached(KSoundID id, class KEntity* ent, const KSoundProperties& props = KSoundProperties());
};


#endif