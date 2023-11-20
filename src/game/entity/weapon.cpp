#include "weapon.h"
#include "engine/game/local_player.h"

#if !_SERVER
#include "properties/renderable.h"
#endif

// DELETE
#include "character_player.h"
#include "properties/powerup_inventory.h"
#include "powerup_index.h"
#include "engine/net/state.h"

void KEntity_Weapon::GetTransient_OwningEntityID(u32& val)
{
	if (KEntity* ent = GetCarryingEntity())
	{
		KSnapshottable* s = ent->As<KSnapshottable>();
		val = s->NetID;
	}
}

void KEntity_Weapon::SetTransient_OwningEntityID(u32& val)
{
	KEntity* ent = dynamic_cast<KEntity*>(GetNetState()->GetReplicatedObject(val));

	if (ent)
	{
		CarryingEntity = ent;
		AddToCarryingInventory();
	}
}

bool KEntity_Weapon::Wep_CanFire(bool primary) const
{
	if (AmmoCount == 0 && RequireAmmoForFire(primary)) return false;
	const KEntity* ent = GetCarryingEntity();
	if (!ent) return false;

	u8 cooldown = GetCooldownState();

	bool cdPrimary = cooldown & EWeaponCooldown::Primary;
	bool cdAlt = cooldown & EWeaponCooldown::Alt;

	switch (CooldownSharing)
	{
		case ECooldownSharing::Shared:
			return !cdPrimary && !cdAlt;
		case ECooldownSharing::Unshared:
			return primary ? !cdPrimary : !cdAlt;
		case ECooldownSharing::PrimaryRequireAlt:
			return primary ? !cdAlt && !cdPrimary : true;
		case ECooldownSharing::AltRequirePrimary:
			return primary ? true : !cdAlt && !cdPrimary;
	}

	return false;
}

void KEntity_Weapon::Tick()
{
	if (KEntity* ent = GetCarryingEntity())
	{
		SetPosition(ent->GetPosition());
	}
}

void KEntity_Weapon::AddToCarryingInventory()
{
	GetCarryingEntity()->As<KEntProp_WeaponInventory>()->CarriedWeapons[WeaponID] = this;
}

bool KEntity_Weapon::CanPrimaryFire() const
{
	return Wep_CanFire(true);
}

bool KEntity_Weapon::CanAltFire() const
{
	return Wep_CanFire(false);
}

void KEntity_Weapon::ConsumeAmmo(u16 count /*= 1*/)
{
	if (!IsNetAuthority()) return;
	count = KMin<u16>(count, AmmoCount);
	AmmoCount -= count;
}

void KEntity_Weapon::AddAmmo(u16 count)
{
	count = KMin<u16>(count, MAX_U16 - AmmoCount);
	AmmoCount += count;
	if (AmmoCount > GetMaxAmmo()) AmmoCount = GetMaxAmmo();
}

bool KEntity_Weapon::IsEquipped() const
{
	if (!CarryingEntity.IsValid()) return false;
	KEntProp_WeaponInventory* inv = CarryingEntity.Get()->As<KEntProp_WeaponInventory>();
	return inv->GetCurrentWeaponIndex() == WeaponID;
}

KEntity* KEntity_Weapon::GetCarryingEntity() const
{
	return CarryingEntity.Get();
}

u8 KEntity_Weapon::GetCooldownState() const
{
	u8 result = EWeaponCooldown::None;

	u32 pc = PrimaryFireCooldown;
	u32 ac = AltFireCooldown;

	if (const KEntProp_PowerupInventory* inv = dynamic_cast<const KEntProp_PowerupInventory*>(GetCarryingEntity()))
	{
		if (inv->GetPowerupFramesRemaining(EPowerupID::Brain) > 0)
		{
			pc /= 6;
			ac /= 6;
			if (pc == 0) pc = 1;
			if (ac == 0) ac = 1;
		}
	}

	if (KTime::FramesSince(LastPrimaryFireFrame) < pc && LastPrimaryFireFrame > 0)
		result |= EWeaponCooldown::Primary;

	if (KTime::FramesSince(LastAltFireFrame) < ac && LastAltFireFrame > 0)
		result |= EWeaponCooldown::Alt;

	return result;
}

void KEntity_Weapon::ForceEndCooldown(u8 cooldown)
{
	u32 frame = KTime::FrameCount();

	switch (cooldown)
	{
		case EWeaponCooldown::Both:
			LastPrimaryFireFrame = frame;
			LastAltFireFrame = frame;
			break;
		case EWeaponCooldown::Primary:
			LastPrimaryFireFrame = frame;
			break;
		case EWeaponCooldown::Alt:
			LastAltFireFrame = frame;
			break;
	}
}

#if !_SERVER
bool KEntity_Weapon::UpdateWeaponRenderBuffer(struct KStaticMeshBase* entry)
{
	if (IsEquipped())
	{
		KEntity* ent = CarryingEntity.Get();
		if (ent == GetViewedEntity())
		{
			GVec3 pos = 0;//GetPosition();
			pos += GVec3(10, 0, -8);

			entry->PrevModelMat = GetLocalPlayer()->WeaponRenderInfo.GetPrevMatrix();
			entry->CurrentModelMat = GetLocalPlayer()->WeaponRenderInfo.BuildMatrix(pos);
			entry->SetLastMoveRenderAlpha(1);
		}
		else
		{
			return false;
			GVec3 pos = ent->GetPosition();
	
			entry->PrevModelMat = glm::translate(glm::mat4(1), pos.ToGLM());
			entry->CurrentModelMat = glm::translate(glm::mat4(1), pos.ToGLM());
			
			entry->SetLastMoveRenderAlpha(1);
		}
		return true;
	}
	return false;
}
#endif

void KEntity_Weapon::GetInputInfo(KFireInputInfo& info)
{
	info.Carrier = GetCarryingEntity();
	if (info.Carrier)
	{
		if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(info.Carrier))
		{
			info.Pitch = cont->GetPitch();
			info.Yaw = cont->GetYaw();
			info.bIsMine = cont->IsViewedEntity();
		}

		info.Forward = GVec3::FromPitchYaw(info.Pitch, info.Yaw);
		info.Right = info.Forward.Cross(GVec3(0, 0, 1));

		info.CamPosition = info.Carrier->GetPosition();
		if (KEntProp_Movable* mover = info.Carrier->As<KEntProp_Movable>())
		  if (KMoveState_Walking* walk = info.Carrier->As<KMoveState_Walking>())
			info.CamPosition.z += 20 - mover->GetCrouchDepth() * walk->CrouchDropDistance;
	}
}
