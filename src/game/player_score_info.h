#pragma once

#include "kfglobal.h"

struct KPlayerScoreInfo
{
	KString PlayerName;
	i32 Score = 0;
	i32 Frags = 0;
	i32 Deaths = 0;
	i32 Damage = 0;
	u32 FrameJoined = 0;
	u32 Ping = 0;
	bool bIsMine = false;

	i32 GetStatByIndex(u32 index) const
	{
		switch (index)
		{
			case 0: return Score;
			case 1: return Frags;
			case 2: return Deaths;
			case 3: return Damage;
			case 4: return Ping;
			//case 4: return FrameJoined;
		}
		return 0;
	}
};
