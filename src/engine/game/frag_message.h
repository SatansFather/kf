#pragma once

#if !_SERVER

#include "kfglobal.h"

struct KFragMessage
{
	f64 TimeCreated = 0;
	KString PlayerName;
	bool bUpdated = true;
	KFragMessage() = default;
	KFragMessage(const KString& name);
};

struct KDeathMessage
{
	f64 TimeCreated = 0;
	u8 DamageType = 0;
	KString PlayerName;
	bool bShouldShow = false;
	bool bUpdated = true;
	KDeathMessage() = default;
	KDeathMessage(const KString& name, u8 damageType);
};

#endif