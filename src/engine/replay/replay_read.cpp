#include "replay_read.h"
#include <fstream>
#include "engine/game_instance.h"
#include "engine/net/state.h"
#include "engine/net/client.h"
#include "engine/game/match.h"
#include "engine/global/paths.h"
#include <filesystem>

KReplayReader* GetReplayReader()
{
	if (KGameMatch* m = GetGameMatch())
		return &m->ReplayReader;

	return nullptr;
}

bool IsReadingReplay()
{
	if (KReplayReader* r = GetReplayReader())
		return r->IsPlaying();
	return false;
}

bool KReplayReader::PlayReplay(const KString& name)
{
	File = std::ifstream(REPLAY_DIR + name.Get() + ".kfdemo", std::ios::in | std::ios::binary);

	if (!File.is_open())
		return false;

	KGameInstance& inst = KGameInstance::Get();
	KMapHash hash;

	// read map hash

	//if (!inst.HasMapWithHash(hash)) // has map
	//	return false;
	//
	//inst.NetDisconnect();
	//inst.LoadMap(hash);

	inst.CreateReplayNetInterface();

	bIsPlaying = true;
	return true;
}

bool KReplayReader::AdvanceFrame()
{
#if !_SERVER
	if (File.peek() == EOF) return false;

	u32 frame = KTime::FrameCount();
	K_ASSERT(frame > LastAdvanceFrame || LastAdvanceFrame == MAX_U32, "called advance frame twice in one frame");
	LastAdvanceFrame = frame; 

	if (KNetState* state = GetNetState())
	{
		if (KNetInterface_Client* c = dynamic_cast<KNetInterface_Client*>(GetNetInterface()))
		{
			u32 size = 0;
			File.read((char*)&size, 4);
			if (size > 0)
			{
				u8* data = new u8[size];
				File.read((char*)data, size);
				c->ProcessGameState(data, size);
				delete[] data;
			}
			return true;
		}
	}
#endif

	return false;
}
