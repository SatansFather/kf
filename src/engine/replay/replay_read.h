#pragma once

#include "kfglobal.h"
#include <fstream>

class KReplayReader
{
	std::ifstream File;
	bool bIsPlaying = false;
	u32 LastAdvanceFrame = MAX_U32;

	TMap<u32, KString> LastListedReplays;

public:

	bool PlayReplay(const KString& name);
	bool IsPlaying() const { return bIsPlaying; }
	bool AdvanceFrame();
};

KReplayReader* GetReplayReader();