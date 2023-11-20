#include "../properties/powerup_inventory.h"
#include "pickup_powerup.h"


void KEntity_Pickup_Powerup::GetTransient_DropDeathFrame(u32& val)
{
	u32 add = 5 * 60;
	u32 age = GetEntityFrameAge();

	if (age <= add)
		add = add - age;
	else
		add = 0;

	val = KTime::FrameCount() + FrameDuration + add;
}

void KEntity_Pickup_Powerup::SetTransient_DropDeathFrame(u32& val)
{
	if (val > KTime::FrameCount())
		FrameDuration = KTime::FrameCount() - val;
	else
		FrameDuration = 0;
}

bool KEntity_Pickup_Powerup::CanPickUp(KEntity* ent)
{
	KEntProp_PowerupInventory* p = dynamic_cast<KEntProp_PowerupInventory*>(ent);
	return p;
}

void KEntity_Pickup_Powerup::PickUp(KEntity* ent)
{
	KEntProp_PowerupInventory* inv = dynamic_cast<KEntProp_PowerupInventory*>(ent);
	TObjRef<KEntity_Powerup>& powerup = inv->CarriedPowerups[PowerupIndex];
	u32 frames = FrameDuration; // 0 unless drop item

	KEntity_Powerup* p = powerup.Get();

	if (p) 
	{
		// already has powerup

		if (frames == 0)
			frames = p->GetFrameDuration();
	}
	else		 
	{
		// new powerup

		powerup = CreatePowerup();
		p = powerup.Get();
		p->LastFramePosition = ent->GetPosition();

		if (IsDropItem())
			p->SetRemainingFrames(0);
	}

	p->AddRemainingFrames(frames);

	p->CarryingEntity = ent;
	if (KSnapshottable* snap = ent->As<KSnapshottable>())
		p->CarryingEntID = snap->GetNetID();
}

void KEntity_Pickup_Powerup::Tick()
{
	KEntity_PickupBase::Tick();

	if (IsDropItem() && FrameDuration > 0)
	{
		if (IsNetClient())
		{
			if (FrameDuration > 0)
				FrameDuration--;
		}
		else if (GetEntityFrameAge() > 5 * 60 && FrameDuration > 0)
		{
			FrameDuration--;
			if (FrameDuration == 0)
				DestroyEntity();
		}
	}
}

