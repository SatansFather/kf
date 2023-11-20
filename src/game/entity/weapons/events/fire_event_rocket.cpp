
// header 
/*
#include "fire_event.h"
#include "engine/net/snapshot.h"
struct KNetSnapshot_FireEventRocket : public KNetSnapshot
{
	SNAPSHOT_PROPERTY(FVec3, Position, FIRST_ONLY)
	SNAPSHOT_PROPERTY(FVec3, Direction, FIRST_ONLY)
	SNAPSHOT_PROPERTY(u32, Frame, FIRST_ONLY)
};

#pragma pack(push, 1)
struct KDestroyedNetObject_FireEventRocket : public KDestroyedNetObject
{
	FVec3 HitNormal;
	FVec3 HitPosition;
	u32 HitObjectNetID = 0; // hit player wont show an explosion

	KDestroyedNetObject_FireEventRocket()
	{
		ParamsSize = 2 * sizeof(FVec3) + sizeof(u32);
	}
};
#pragma pack(pop)

class KEntity_FireEvent_Rocket : public KEntity_FireEvent,
	public TSnapshottable<KNetSnapshot_FireEventRocket,
{

};*/




/*
#include "wep_rocket.h"
#include "../projectiles/proj_rocket.h"
#include "../properties/controllable.h"
#include "../properties/powerup_inventory.h"
#include "../graphics/flash.h"
#include "engine/game/local_player.h"
#include "engine/audio/audio.h"
#include "engine/net/state.h"

template <>
KSnapshottable* TSnapshottable<KNetSnapshot_WeaponRocket>::InstantiateObject()
{
	return TDataPool<KEntity_Weapon_Rocket>::GetPool()->CreateNew().Get();
}

KEntity_Weapon_Rocket::KEntity_Weapon_Rocket()
{
	WeaponID = EWeaponID::Rocket;
	PrimaryFireCooldown = KTime::FramesFromTime(.6);
	AltFireCooldown = KTime::FramesFromTime(.4);
}

void KEntity_Weapon_Rocket::UnpackSnapshot()
{
	SetAmmoCount(ClientData.Ammo);

	if (GetNetState())
	{
		CarryingEntity = dynamic_cast<KEntity*>
			(GetNetState()->GetReplicatedObject(ClientData.OwningEntityID));
		AddToCarryingInventory();
	}
}

void KEntity_Weapon_Rocket::CreateSnapshot(KNetSnapshot_WeaponRocket& snapshot)
{
	snapshot.Ammo = GetAmmoCount();

	if (KEntity* ent = GetCarryingEntity())
	{
		KSnapshottable* s = ent->As<KSnapshottable>();
		snapshot.OwningEntityID = s->NetID;
	}
}

bool KEntity_Weapon_Rocket::Fire()
{
	KEntity* guy = GetCarryingEntity();
	GFlt pitch = 0, yaw = 0;
	bool mine = false;
	if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(guy))
	{
		pitch = cont->GetPitch();
		yaw = cont->GetYaw();
		mine = cont->IsViewedEntity();
	}

	const GVec3 dir = GVec3::FromPitchYaw(pitch, yaw);
	const GVec3 right = dir.Cross(GVec3(0, 0, 1));

	KEntity_Projectile_Rocket* proj =
		TDataPool<KEntity_Projectile_Rocket>::GetPool()->CreateNew().Get();
	proj->SetPosition(guy->GetPosition().AdjustZ(20));
	proj->FiringWeapon = this;
	proj->Velocity = dir * 800;


#if !_SERVER
	proj->Trail = KEntity_RocketTrail::Create(proj->GetPosition(), proj->Velocity, true, mine);
	proj->RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
	proj->BuildMatrix(proj->PrevMatrix);
#endif

	if (KEntProp_CollidableBBox* box = dynamic_cast<KEntProp_CollidableBBox*>(guy))
		proj->AssignIgnoreID(box->IgnoreID);

	proj->OwningPlayerIndex = OwningPlayerIndex;

/ *#if !_SERVER
	proj->RenderOffset = GVec3(0, 0, -12).GetRotated(rot, right);
	proj->SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
	KEntity_ShotgunTrail* beam = proj->SmokeBeam.Get();
	beam->StartPos = proj->GetPosition() + proj->RenderOffset + (shootDir * 24);
	beam->LastEndPos = beam->StartPos;
#endif* /


	//ConsumeAmmo();

	return true;
}

bool KEntity_Weapon_Rocket::AltFire()
{
	KEntity* guy = GetCarryingEntity();
	GFlt pitch = 0, yaw = 0;
	bool mine = false;
	if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(guy))
	{
		pitch = cont->GetPitch();
		yaw = cont->GetYaw();
		mine = cont->IsViewedEntity();
	}

	const GVec3 dir = GVec3::FromPitchYaw(pitch, yaw);
	const GVec3 right = dir.Cross(GVec3(0, 0, 1));

	KEntity_Projectile_Rocket* proj =
		TDataPool<KEntity_Projectile_Rocket>::GetPool()->CreateNew().Get();
	proj->SetPosition(guy->GetPosition().AdjustZ(20));
	proj->FiringWeapon = this;
	proj->Velocity = dir * 1500;

	proj->DirectDamage = 50;
	proj->MaxSplashDamage = 30;
	proj->SplashRadius = 64;
	proj->PushScale = 200;

#if !_SERVER
	proj->Trail = KEntity_RocketTrail::Create(proj->GetPosition(), proj->Velocity, true, mine);
	proj->RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
	proj->RenderOffsetAdjustRate = 1.2;
	proj->BuildMatrix(proj->PrevMatrix);
#endif

	if (KEntProp_CollidableBBox* box = dynamic_cast<KEntProp_CollidableBBox*>(guy))
		proj->AssignIgnoreID(box->IgnoreID);

	proj->OwningPlayerIndex = OwningPlayerIndex;

	/ *#if !_SERVER
		proj->RenderOffset = GVec3(0, 0, -12).GetRotated(rot, right);
		proj->SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
		KEntity_ShotgunTrail* beam = proj->SmokeBeam.Get();
		beam->StartPos = proj->GetPosition() + proj->RenderOffset + (shootDir * 24);
		beam->LastEndPos = beam->StartPos;
	#endif* /


	//ConsumeAmmo();

	return true;
}

void KEntity_Weapon_Rocket::PrimaryFireCosmetics()
{
#if !_SERVER
	KEntity* guy = GetCarryingEntity();
	KEntProp_Controllable* cont = guy->As<KEntProp_Controllable>();
	KEntProp_PowerupInventory* powerup = guy->As<KEntProp_PowerupInventory>();

	//if (powerup && powerup->HasPowerup(EPowerupID::Brain))
	//{
	//	GetLocalPlayer()->WeaponRenderInfo.SetRecoil(2);
	//	GetLocalPlayer()->PushCameraFov(1, .2, .2);
	//	GetLocalPlayer()->ShakeCamera(1, .3, 16);
	//	GetLocalPlayer()->PushCameraRotation(FVec3(2, 0, 0), .15, .5);
	//	GetLocalPlayer()->AddConcussion(.4);
	//}
	//else
	{
		if (cont->IsViewedEntity())
		{
			GetLocalPlayer()->WeaponRenderInfo.SetRecoil(4);
			GetLocalPlayer()->PushCameraFov(2, .2, .2);
			GetLocalPlayer()->ShakeCamera(.5, .5, 16);
			GetLocalPlayer()->PushCameraRotation(FVec3(1, 0, 0), .15, .35);
			GetLocalPlayer()->AddConcussion(.5);
		}
	}
	f32 radius = cont && cont->IsViewedEntity() ? 1024 : 192;

	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(127, 40, 16, 255), radius, .2, .5, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);
#endif
}

void KEntity_Weapon_Rocket::AltFireCosmetics()
{

}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Rocket::UpdateBuffers(KStaticMesh<"gunplaceholder", "rockettex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true / * is viewed * /) FlagBufferForViewWeapon(entry);
	return KBufferUpdateResult(true, vis);
}
#endif
*/
