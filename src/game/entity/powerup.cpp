#include "powerup.h"
#include "properties/powerup_inventory.h"
#include "character_player.h"
#include "engine/net/state.h"

void KEntity_Powerup::GetTransient_EndFrame(u32& val)
{
	val = KTime::FrameCount() + GetRemainingFrames();
}

void KEntity_Powerup::SetTransient_EndFrame(u32& val)
{
	u32 frame = KTime::FrameCount();
	if (val < frame) val = frame;
	RemainingFrames = val - KTime::FrameCount();
}

void KEntity_Powerup::InitNetObject()
{
	KSnapshottable* snap = GetNetState()->GetReplicatedObject(CarryingEntID);
	if (KEntity* ent = dynamic_cast<KEntity*>(snap))
	{
		CarryingEntity = ent;
		if (KEntProp_PowerupInventory* inv = ent->As<KEntProp_PowerupInventory>())
			inv->CarriedPowerups[PowerupID] = this;
	}
}

KEntProp_PowerupInventory* KEntity_Powerup::GetInventory()
{
	if (CarryingEntity.IsValid())
		return CarryingEntity.As<KEntProp_PowerupInventory>();

	return nullptr;
}

void KEntity_Powerup::Tick()
{
	if (KEntity* guy = CarryingEntity.Get())
		SetPosition(guy->GetPosition());

	if (RemainingFrames > 0) RemainingFrames--;
	if (RemainingFrames == 0) DestroyEntity();
}
