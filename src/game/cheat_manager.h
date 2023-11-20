#pragma once

#include "kfglobal.h"

struct EPersistentCheat
{
	enum
	{
		God				= 0x1,
		Fly				= 0x2,
		Ghost			= 0x4,
		Ammo			= 0x8,

		Powerup_Brain	= 0x10,
		Powerup_Rage	= 0x20,
	};
};

class KCheatManager
{
	static u32 PersistentCheats;

public:

	static void AddPersistentCheat(u32 cheat);
	static void RemovePersistentCheat(u32 cheat);
	static void TogglePersistentCheat(u32 cheat);
	static void ClearPersistentCheats();
	
	static bool CheatIsActive(u32 cheat);

	static void GiveAllGuns() {}
};