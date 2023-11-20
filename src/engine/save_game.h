#pragma once

#include "kfglobal.h"

class KSaveGame
{
	u32 FrameNumber;

public:
	
	void SaveState();
	void WriteToFile(const KString& fileName);
};