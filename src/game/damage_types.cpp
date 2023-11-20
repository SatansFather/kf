#include "damage_types.h"

KDamageType KDamageType::Get(EDamageType type)
{
	switch (type)
	{
		case EDamageType::ShotgunPrimary:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
			return dmg;
		}
		case EDamageType::ShotgunAlt:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::RocketPrimary:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::RocketAlt:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::ZapperPimary:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::ZapperAlt:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::Fall:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::Lava:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
		case EDamageType::Spikes:
		{
			static KDamageType dmg;
			dmg.KillMessage;
			dmg.SuicideMessage;
		}
	}

	static KDamageType gen;
	return gen;
}

KDamageType KDamageType::GetCustom(const KString& type)
{
	KDamageType t;
	return t;
}
