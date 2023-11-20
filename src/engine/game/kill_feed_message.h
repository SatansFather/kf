#pragma once

#if !_SERVER

#include "kfglobal.h"

struct KKillFeedMessage
{
	KString KillerName;
	KString VictimName;
	f64 TimeAdded = 0;
	u8 DamageType = 0;

	KKillFeedMessage() = default;
	KKillFeedMessage(const KString& killer, const KString& victim, u8 damageType);
};

#endif