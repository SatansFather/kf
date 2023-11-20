#include "wep_rocket.h"
#include "../projectiles/proj_rocket.h"
#include "../properties/controllable.h"
#include "../properties/powerup_inventory.h"
#include "../graphics/flash.h"
#include "engine/game/local_player.h"
#include "engine/audio/audio.h"
#include "engine/net/state.h"
#include "engine/game_instance.h"
#include "engine/net/player.h"

KEntity_Weapon_Rocket::KEntity_Weapon_Rocket()
{
	WeaponID = EWeaponID::Rocket;
	PrimaryFireCooldown = KTime::FramesFromTime(.6);
	AltFireCooldown = KTime::FramesFromTime(.4);
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

	KProjectileCreationParams params;
	params.Position = guy->GetPosition();

	if (KEntity_Character_Player* player = guy->As<KEntity_Character_Player>())
		params.Position.z += 20 - player->GetCrouchDepth() * player->CrouchDropDistance;

	params.Direction = dir;
	params.Speed = 1000;
	params.FiringWeapon = this;
	params.OwningPlayerIndex = OwningPlayerIndex;
	params.bPrimaryFire = true;
#if !_SERVER
	params.RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
#endif

	KEntity_Projectile_Rocket* proj = KEntity_Projectile_Rocket::Create(params).Get();

	proj->MaxSplashDamage = 200;
	proj->SplashRadius = 200;

/*#if !_SERVER
	proj->RenderOffset = GVec3(0, 0, -12).GetRotated(rot, right);
	proj->SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
	KEntity_ShotgunTrail* beam = proj->SmokeBeam.Get();
	beam->StartPos = proj->GetPosition() + proj->RenderOffset + (shootDir * 24);
	beam->LastEndPos = beam->StartPos;
#endif*/


	ConsumeAmmo();

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

	KProjectileCreationParams params;
	params.Position = guy->GetPosition();

	if (KEntity_Character_Player* player = guy->As<KEntity_Character_Player>())
		params.Position.z += 20 - player->GetCrouchDepth() * player->CrouchDropDistance;

	params.Direction = dir;
	params.Speed = 1600;
	params.FiringWeapon = this;
	params.OwningPlayerIndex = OwningPlayerIndex;
	params.bPrimaryFire = false;
#if !_SERVER
	params.RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
#endif

	KEntity_Projectile_Rocket* proj = KEntity_Projectile_Rocket::Create(params).Get();

	proj->DirectDamage = 50;
	proj->MaxSplashDamage = 30;
	proj->SplashRadius = 64;
	proj->PushScale = 200;


	/*#if !_SERVER
		proj->RenderOffset = GVec3(0, 0, -12).GetRotated(rot, right);
		proj->SmokeBeam = TDataPool<KEntity_ShotgunTrail>::GetPool()->CreateNew();
		KEntity_ShotgunTrail* beam = proj->SmokeBeam.Get();
		beam->StartPos = proj->GetPosition() + proj->RenderOffset + (shootDir * 24);
		beam->LastEndPos = beam->StartPos;
	#endif*/


	ConsumeAmmo();

	return true;
}

void KEntity_Weapon_Rocket::PrimaryFireCosmetics()
{
	KEntity* guy = GetCarryingEntity();
#if !_SERVER
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
	f32 radius = cont && cont->IsViewedEntity() ? 1024 * KGameInstance::Get().bWeaponFlash : 192;

	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(127, 40, 16, 255), radius, .2, .5, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);

	if (guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Rocket_Primary, 1);
	}
#endif
	if (IsNetServer() && guy)
	{
		KSoundProperties props;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Rocket_Primary, guy->GetPosition(), props);
	}

}

void KEntity_Weapon_Rocket::AltFireCosmetics()
{

}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Rocket::UpdateBuffers(KStaticMesh<"gunplaceholder", "rockettex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true /* is viewed */) FlagBufferForViewWeapon(entry);
	return KBufferUpdateResult(true, vis);
}
#endif
