#include "map_pickup.h"
#include "../pickups/powerups/pickup_powerup_brain.h"
#include "../pickups/pickup_health.h"
#include "../pickups/powerups/pickup_powerup_invis.h"
#include "../pickups/pickup_weapon.h"
#include "../weapon_index.h"

void KMapEntity_PickupItem::CreatePickup(const KString& name)
{
	if (name.Get() == "item_powerup_fly")
	{
		TDataPool<KEntity_Pickup_Powerup_Brain>::GetPool()->CreateNew().Get()->SetPosition(GetMapPosition().AdjustZ(.1));
	}
	if (name.Get() == "item_powerup_invis")
	{
		TDataPool<KEntity_Pickup_Powerup_Invis>::GetPool()->CreateNew().Get()->SetPosition(GetMapPosition().AdjustZ(.1));
	}
	else if (name.Get() == "item_health_50")
	{
		KEntity_Pickup_Health* h = TDataPool<KEntity_Pickup_Health>::GetPool()->CreateNew().Get();
		h->SetPosition(GetMapPosition().AdjustZ(.1));
		h->SetHealthType(EReppedPickupFlags::HP50);
	}
	else if (name.Get() == "item_health_100")
	{
		KEntity_Pickup_Health* h = TDataPool<KEntity_Pickup_Health>::GetPool()->CreateNew().Get();
		h->SetPosition(GetMapPosition().AdjustZ(.1));
		h->SetHealthType(EReppedPickupFlags::HP100);
	}
	else if (name.Get() == "item_health_200")
	{
		KEntity_Pickup_Health* h = TDataPool<KEntity_Pickup_Health>::GetPool()->CreateNew().Get();
		h->SetPosition(GetMapPosition().AdjustZ(.1));
		h->SetHealthType(EReppedPickupFlags::HP200);
	}
	else if (name.Get() == "item_weapon_shotgun")
	{
		auto w = TDataPool<KEntity_Pickup_Weapon_Shotgun>::GetPool()->CreateNew().Get();
		//w->WeaponID = EWeaponID::Shotgun;
		w->SetPosition(GetMapPosition().AdjustZ(.1));
	}
	else if (name.Get() == "item_weapon_rocket")
	{
		auto w = TDataPool<KEntity_Pickup_Weapon_Rocket>::GetPool()->CreateNew().Get();
		//w->WeaponID = EWeaponID::Rocket;
		w->SetPosition(GetMapPosition().AdjustZ(.1));
	}
	else if (name.Get() == "item_weapon_cannon")
	{
		auto w = TDataPool<KEntity_Pickup_Weapon_Cannon>::GetPool()->CreateNew().Get();
		//w->WeaponID = EWeaponID::Cannon;
		w->SetPosition(GetMapPosition().AdjustZ(.1));
	}
	else if (name.Get() == "item_weapon_blaster")
	{
		auto w = TDataPool<KEntity_Pickup_Weapon_Blast>::GetPool()->CreateNew().Get();
		//w->WeaponID = EWeaponID::Cannon;
		w->SetPosition(GetMapPosition().AdjustZ(.1));
	}
}
