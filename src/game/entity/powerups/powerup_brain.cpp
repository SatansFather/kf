#include "powerup_brain.h"
#include "../properties/movable.h"
#include "engine/game/local_player.h"
#include "engine/net/state.h"
#include "../weapon.h"
#include "powerup_invis.h"

KEntity_Powerup_Brain::KEntity_Powerup_Brain()
{
	PowerupID = EPowerupID::Brain;
}

void KEntity_Powerup_Brain::Tick()
{
	if (CarrySound.GetHandle() == 0 && CarryingEntity.IsValid())
	{
		KSoundProperties props;
		props.bLooping = true;
		props.Volume = 1.3;
		props.MaxDistance = 4096;
		CarrySound = KAudio::PlaySoundAttached(KSoundID::Carry_Brain, CarryingEntity.Get(), props);
	}
	
	bool invis = false;
	if (KEntity* ent = CarryingEntity.Get())
	  if (GetControlledEntity() == ent)
	    if (KEntProp_PowerupInventory* inv = ent->As<KEntProp_PowerupInventory>())
	      if (inv->HasPowerup(EPowerupID::Invis))
	        if (dynamic_cast<KEntity_Powerup_Invis*>(inv->CarriedPowerups[EPowerupID::Invis].Get())->IsInvisible())
		      invis = true;

	if (invis)
		KAudio::SetHandleVolume(CarrySound, 0);
	else
		KAudio::SetHandleVolume(CarrySound, 1.3);

	if (IsNetAuthority());
	  if (GetEntityFrameAge() % 15 == 0)
	    if (KEntity* ent = CarryingEntity.Get())
	      if (KEntProp_WeaponInventory* inv = ent->As<KEntProp_WeaponInventory>())
	        for (u8 i = 0; i < EWeaponID::NumWeapons; i++)
	          if (KEntity_Weapon* wep = inv->GetWeaponByIndex(i))
	            if (wep->GetCooldownState() == EWeaponCooldown::None)
	              wep->AddAmmo(wep->GetRegenCount());

	KEntity_Powerup::Tick();
}

void KEntity_Powerup_Brain::OnEntityDestroyed()
{
	KAudio::StopSound(CarrySound);

	if (KEntity* ent = CarryingEntity.Get())
	{
		if (ent == GetViewedEntity())
			KAudio::PlaySoundAttached(KSoundID::Drop_Brain, ent);
		else
			KAudio::PlaySound3D(KSoundID::Drop_Brain, GetPosition());
	}
}

#if !_SERVER
KBufferUpdateResult KEntity_Powerup_Brain::UpdateBuffers(KDynamicLight& entry)
{
	if (CarryingEntity.IsValid())
	{
		bool invis = false;
		if (KEntity* ent = CarryingEntity.Get())
		  if (KEntProp_PowerupInventory* inv = ent->As<KEntProp_PowerupInventory>())
		    if (inv->HasPowerup(EPowerupID::Invis))
		      if (dynamic_cast<KEntity_Powerup_Invis*>(inv->CarriedPowerups[EPowerupID::Invis].Get())->IsInvisible())
		        invis = true;
		
		if (invis)
			return false;

		KEntity* guy = CarryingEntity.Get();
	//	SetPosition(guy->GetPosition());
		entry.SetPrevPosition(LastFramePosition);
		entry.SetCurrentPosition(GetPosition());

		entry.SetFalloff(1);
		entry.SetColor(FColor8(255, 100, 255, 255));

		f32 prevAge = KSaturate((GetEntityAge() - GameFrameDelta()) * 2);
		f32 age = KSaturate(GetEntityAge() * 2);

		entry.SetPrevRadius(prevAge * 192);
		entry.SetCurrentRadius(age * 192);

		LastFramePosition = GetPosition();

		return true;
	}

	return false;
}
#endif