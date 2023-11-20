#pragma once

#include "kfglobal.h"

enum class EDamageType
{
	Generic,

	// weapons
	ShotgunPrimary,
	ShotgunAlt,
	RocketPrimary,
	RocketAlt,
	ZapperPimary,
	ZapperAlt,

	// world
	Fall,
	Lava,
	Spikes,
};

struct KDamageType
{
	KString KillMessage = "%k killed %v";
	KString SuicideMessage = "%v died";

	static KDamageType Get(EDamageType type);
	static KDamageType GetCustom(const KString& type);
};
