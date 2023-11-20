#include "deferred_drop.h"
#include "../../engine/game/match.h"
#include "pickups/pickup_powerup.h"
#include "pickups/powerups/pickup_powerup_invis.h"
#include "pickups/powerups/pickup_powerup_brain.h"
#include "pickups/pickup_health.h"
#include "pickups/pickup_weapon.h"

KEntity_Pickup_Powerup* SpawnDropPowerup(u8 index)
{
	switch (index)
	{
		case EPowerupID::Brain:
		{
			return TDataPool<KEntity_Pickup_Powerup_Brain>::GetPool()->CreateNew().Get();
		}
		case EPowerupID::Invis:
		{
			return TDataPool<KEntity_Pickup_Powerup_Invis>::GetPool()->CreateNew().Get();
		}
	}

	return nullptr;
}

KEntity_Pickup_Weapon* SpawnDropWeapon(u8 index)
{
	switch (index)
	{
		case EWeaponID::Shotgun:
		{
			return TDataPool<KEntity_Pickup_Weapon_Shotgun>::GetPool()->CreateNew().Get();
		}
		case EWeaponID::Rocket:
		{
			return TDataPool<KEntity_Pickup_Weapon_Rocket>::GetPool()->CreateNew().Get();
		}
		case EWeaponID::Cannon:
		{
			return TDataPool<KEntity_Pickup_Weapon_Cannon>::GetPool()->CreateNew().Get();
		}
		case EWeaponID::Zapper:
		{
		//	return TDataPool<KEntity_Pickup_Weapon_Zapper>::GetPool()->CreateNew().Get();
		}
		case EWeaponID::Blast:
		{
			//	return TDataPool<KEntity_Pickup_Weapon_Blast>::GetPool()->CreateNew().Get();
		}
	}

	return nullptr;
}

void KDeferredDropSpawn::Spawn()
{
	if (!IsNetAuthority()) return;

	switch (ItemType)
	{
		case Type_Health:
		{
			if (KEntity_Pickup_Health* h = TDataPool<KEntity_Pickup_Health>::GetPool()->CreateNew().Get())
			{
				h->SetPosition(Position);
				h->SetHealthType(HealthFlags);
				h->InitDropItem();
				break;
			}
		}
		case Type_Powerup:
		{
			if (KEntity_Pickup_Powerup* p = SpawnDropPowerup(PowerupIndex))
			{
				p->SetPosition(Position);
				p->FrameDuration = FrameDuration;
				p->InitDropItem();
				break;
			}
		}
		case Type_Weapon:
		{
			if (KEntity_Pickup_Weapon* w = SpawnDropWeapon(WeaponIndex))
			{
				w->SetPosition(Position);
				w->InitDropItem();
				break;
			}
			break;
		}
	}
}

void KDeferredDropSpawn::Finalize()
{
	GetGameMatch()->PendingDropSpawns.push_back(*this);
}
