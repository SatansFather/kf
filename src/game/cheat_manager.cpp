#include "cheat_manager.h"
#include "engine/net/net_interface.h"

extern KString CCOM_Ghost(const KString& val)
{
	KCheatManager::TogglePersistentCheat(EPersistentCheat::Ghost);
	KCheatManager::RemovePersistentCheat(EPersistentCheat::Fly);
	return "Ghost Mode " + KString(KCheatManager::CheatIsActive(EPersistentCheat::Ghost) ? "activated" : "deactivated");
}

extern KString CCOM_Fly(const KString& val)
{
	KCheatManager::TogglePersistentCheat(EPersistentCheat::Fly);
	KCheatManager::RemovePersistentCheat(EPersistentCheat::Ghost);
	return "Fly Mode " + KString(KCheatManager::CheatIsActive(EPersistentCheat::Fly) ? "activated" : "deactivated");
}

extern KString CCOM_Walk(const KString& val)
{
	if (KCheatManager::CheatIsActive(EPersistentCheat::Ghost) || KCheatManager::CheatIsActive(EPersistentCheat::Fly))
	{
		KCheatManager::RemovePersistentCheat(EPersistentCheat::Ghost);
		KCheatManager::RemovePersistentCheat(EPersistentCheat::Fly);
		return "Fly/Ghost Mode deactivated";
	}
	return "";
}

extern KString CCOM_God(const KString& val)
{
	KCheatManager::TogglePersistentCheat(EPersistentCheat::God);
	return "God Mode " + KString(KCheatManager::CheatIsActive(EPersistentCheat::God) ? "activated" : "deactivated");
}

extern KString CCOM_Ammo(const KString& val)
{
	KCheatManager::TogglePersistentCheat(EPersistentCheat::Ammo);
	return "Infinite Ammo " + KString(KCheatManager::CheatIsActive(EPersistentCheat::Ammo) ? "activated" : "deactivated");
}

u32 KCheatManager::PersistentCheats = 0;

void KCheatManager::AddPersistentCheat(u32 cheat)
{
	if (!GetNetInterface())
		PersistentCheats |= cheat;
}

void KCheatManager::RemovePersistentCheat(u32 cheat)
{
	PersistentCheats &= ~cheat;	
}

void KCheatManager::TogglePersistentCheat(u32 cheat)
{
	PersistentCheats ^= cheat;
}

void KCheatManager::ClearPersistentCheats()
{
	PersistentCheats = 0;
}

bool KCheatManager::CheatIsActive(u32 cheat)
{
	return PersistentCheats & cheat;
}
