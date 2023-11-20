#if !_SERVER

#include "sound_asset.h"

#include "../game_instance.h"
#if _PACK
#else
#include "../kwad/kwad_chunk_sound.h"
#endif
#include "../kwad/kwad_reader.h"

void KSoundAsset::LoadWave(const KString& file)
{
#if _PACK
	// add to wad file
	KGameInstance::Get().MainWadFile.AddSound(file);
#elif _DEV
	// load from disk
	Wave.load(KString(SOUND_DIR + file + ".wav"));
#else
	if (!KGameInstance::Get().MainWadFile.IndexedData[EWadFileChunkType::Sound].contains(file))
		return;

	// load from wad
	KWadReader<KWadChunk_Sound> reader("kfdata", file);
	KWadChunk_Sound* sound = reader.GetChunk();
	
	// TODO sound is nullptr
	Wave.loadMem((u8*)sound->Data.data(), sound->Data.size(), false, false);
#endif
}

#endif