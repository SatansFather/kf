#include "sound_list.h"

TVector<UPtr<KSoundAsset>> KSoundID::SoundList;

KSoundAsset* GetSoundAsset(KSoundID id)
{
	return KSoundID::SoundList[id.Index].get();
}
