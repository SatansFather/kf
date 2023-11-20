#include "wep_shotgun.h"
#include "engine/game/local_player.h"
#include "engine/audio/audio.h"
#include "../graphics/flash.h"
#include "engine/utility/random.h"
#include "../projectiles/proj_shotgun.h"

// DELETE
#include "../character_player.h"
#include "../../../engine/net/state.h"
#include "../../../engine/game_instance.h"
#include "engine/net/player.h"
#include "../projectiles/proj_shotgun_boulder.h"

KEntity_Weapon_Shotgun::KEntity_Weapon_Shotgun()
{
	WeaponID = EWeaponID::Shotgun;
	PrimaryFireCooldown = KTime::FramesFromTime(.65);
	AltFireCooldown = KTime::FramesFromTime(.65);
}

bool KEntity_Weapon_Shotgun::Fire()
{
	KEntity* guy = GetCarryingEntity();
	if (!guy) return false;

	GFlt pitch = 0, yaw = 0;
	if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(guy))
	{
		pitch = cont->GetPitch();
		yaw = cont->GetYaw();
	}

	const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
	const GVec3 right = dir.Cross(GVec3(0, 0, 1));

	for (i32 i = -15; i <= 15; i += 2)
	{
		GFlt offset = i + D_RandRange(-.5, .5);
		GVec3 shoot = (dir * 150) + (right * offset);
		GFlt rot = pitch + (D_RandRange(-3, 3) * PI<GFlt>() / 180);
		GVec3 shootDir = shoot.GetRotated(rot, right).GetNormalized();

		KProjectileCreationParams params;
		params.Position = guy->GetPosition();

		if (KEntity_Character_Player* player = guy->As<KEntity_Character_Player>())
			params.Position.z += 20 - player->GetCrouchDepth() * player->CrouchDropDistance;

		params.Direction = shootDir;
		params.Speed = 5000;
		params.FiringWeapon = this;
		params.OwningPlayerIndex = OwningPlayerIndex;
		params.bPrimaryFire = true;
#if !_SERVER
		params.RenderOffset = GVec3(0, 0, -12).GetRotated(rot, right);
#endif
		KEntity_Projectile_ShotgunShard* proj = KEntity_Projectile_ShotgunShard::Create(params).Get();
	}

	ConsumeAmmo();

	return true;
}

bool KEntity_Weapon_Shotgun::AltFire()
{
	KEntity* guy = GetCarryingEntity();
	GFlt pitch = 0, yaw = 0;
	if (KEntProp_Controllable* cont = dynamic_cast<KEntProp_Controllable*>(guy))
	{
		pitch = cont->GetPitch();
		yaw = cont->GetYaw();
	}

	const GVec3 dir = GVec3::FromPitchYaw(0, yaw);
	const GVec3 right = dir.Cross(GVec3(0, 0, 1));
	//GFlt offset = D_RandRange(-1, 1);
	GVec3 shoot = (dir * 100);// + (right * offset);
	GFlt rot = pitch;// + (D_RandRange(-.8, .8) * PI<GFlt>() / 180);
	GVec3 shootDir = shoot.GetRotated(rot, right).GetNormalized();

	KProjectileCreationParams params;
	params.Position = guy->GetPosition();

	if (KEntity_Character_Player* player = guy->As<KEntity_Character_Player>())
		params.Position.z += 10 - player->GetCrouchDepth() * player->CrouchDropDistance;

	params.Direction = shootDir;
	params.Speed = 1200;
	params.FiringWeapon = this;
	params.OwningPlayerIndex = OwningPlayerIndex;
	params.bPrimaryFire = false;
#if !_SERVER
	params.RenderOffset = GVec3(0, 0, -12);//.GetRotated(rot, right);
#endif
	KEntity_Projectile_ShotgunBoulder* proj = KEntity_Projectile_ShotgunBoulder::Create(params).Get();

	ConsumeAmmo();

	return true;
}

void KEntity_Weapon_Shotgun::PrimaryFireCosmetics()
{
	KEntity* guy = GetCarryingEntity();
#if !_SERVER
	KEntProp_Controllable* cont = guy->As<KEntProp_Controllable>();
	KEntProp_PowerupInventory* powerup = guy->As<KEntProp_PowerupInventory>();

	if (powerup && powerup->HasPowerup(EPowerupID::Brain))
	{
		if (cont->IsViewedEntity())
		{
			GetLocalPlayer()->WeaponRenderInfo.SetRecoil(2);
			GetLocalPlayer()->PushCameraFov(1, .2, .2);
			GetLocalPlayer()->ShakeCamera(1, .3, 16);
			GetLocalPlayer()->PushCameraRotation(FVec3(2, 0, 0), .15, .5);
			GetLocalPlayer()->AddConcussion(.4);
		}
	}
	else
	{
		if (cont->IsViewedEntity())
		{
			GetLocalPlayer()->WeaponRenderInfo.SetRecoil(4);
			GetLocalPlayer()->PushCameraFov(2, .2, .2);
			GetLocalPlayer()->ShakeCamera(3, .3, 16);
			GetLocalPlayer()->PushCameraRotation(FVec3(4, 0, 0), .15, .5);
			GetLocalPlayer()->AddConcussion(1);
		}
	}

	f32 radius = cont && cont->IsViewedEntity() ? 1024 * KGameInstance::Get().bWeaponFlash : 192;
	
	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(255, 179, 127, 255), radius, .07, .3, 1).Get();
	flash->PositionEntity = this;
	if (guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Shotgun_Primary, 1);
	}
#endif
	if (IsNetServer() && guy)
	{
		KSoundProperties props;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Shotgun_Primary, guy->GetPosition(), props);
	}
}

void KEntity_Weapon_Shotgun::AltFireCosmetics()
{
	KEntity* guy = GetCarryingEntity();
#if !_SERVER
	KEntProp_Controllable* cont = guy->As<KEntProp_Controllable>();

	if (cont->IsViewedEntity())
	{
		GetLocalPlayer()->WeaponRenderInfo.SetRecoil(1);
		GetLocalPlayer()->PushCameraFov(1, .1, .2);
		GetLocalPlayer()->ShakeCamera(.5, .1, 32);
		//GetLocalPlayer()->PushCameraRotation(FVec3(2, 0, 0), .1, .5);
		GetLocalPlayer()->AddConcussion(.2);
	}

	f32 radius = cont && cont->IsViewedEntity() ? 512 * KGameInstance::Get().bWeaponFlash : 96;
	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(200, 150, 100, 255), radius, .07, .28, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);
	if (guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Shotgun_Alt, 1);
	}
#endif
	if (IsNetServer() && guy)
	{
		KSoundProperties props;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Shotgun_Alt, guy->GetPosition(), props);
	}
}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Shotgun::UpdateBuffers(KStaticMesh<"gunplaceholder", "shotguntex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true /* is viewed */) 
		FlagBufferForViewWeapon(entry);
	
	return KBufferUpdateResult(true, vis);
}
#endif