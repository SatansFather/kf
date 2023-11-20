#if !_SERVER

#include "kill_feed_message.h"
#include "../game_instance.h"
#include "match.h"

KKillFeedMessage::KKillFeedMessage(const KString& killer, const KString& victim, u8 damageType)
{
	if (KGameMatch* m = GetGameMatch())
	{
		KillerName = killer;
		VictimName = victim;
		DamageType = damageType;
		TimeAdded = KGameInstance::Get().GetTotalRenderTime();
		m->NewKillFeedMessages.push_back(*this);
	}
}

#endif
