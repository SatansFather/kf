#if !_SERVER

#include "frag_message.h"
#include "match.h"
#include "../game_instance.h"

KFragMessage::KFragMessage(const KString& name)
{
	TimeCreated = KGameInstance::Get().GetTotalRenderTime();
	PlayerName = name;
	bUpdated = true;
	GetGameMatch()->CurrentFragMessage = *this;
}

KDeathMessage::KDeathMessage(const KString& name, u8 damageType)
{
	TimeCreated = KGameInstance::Get().GetTotalRenderTime();
	PlayerName = name;
	DamageType = damageType;
	bShouldShow = true;
	bUpdated = true;
	GetGameMatch()->CurrentDeathMessage = *this;
}

#endif