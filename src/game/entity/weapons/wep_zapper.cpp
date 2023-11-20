#include "wep_zapper.h"
#include "engine/game/local_player.h"
#include "../properties/controllable.h"
#include "engine/audio/audio.h"
#include "../graphics/flash.h"
#include "../graphics/lightning_bolt.h"

KEntity_Weapon_Zapper::KEntity_Weapon_Zapper()
{
	WeaponID = EWeaponID::Zapper;
	PrimaryFireCooldown = KTime::FramesFromTime(.65);
	AltFireCooldown = KTime::FramesFromTime(.1);
}

bool KEntity_Weapon_Zapper::Fire()
{
	//KEntity_LightningBolt::Create(0, 1024, 4);
	return true;
}

bool KEntity_Weapon_Zapper::AltFire()
{
	return true;
}

void KEntity_Weapon_Zapper::PrimaryFireCosmetics()
{
#if !_SERVER
	GetLocalPlayer()->WeaponRenderInfo.SetRecoil(4);
	GetLocalPlayer()->PushCameraFov(2, .2, .2);
	GetLocalPlayer()->ShakeCamera(3, .2, 16);
	GetLocalPlayer()->PushCameraRotation(FVec3(4, 0, 0), .1, .5);
	GetLocalPlayer()->AddConcussion(1);

	KEntity* guy = GetCarryingEntity();
	KEntProp_Controllable* cont = guy->As<KEntProp_Controllable>();
	f32 radius = cont && cont->IsViewedEntity() ? 1024 : 192;
	
	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(255, 179, 127, 255), radius, .07, .3, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);
#endif
}

void KEntity_Weapon_Zapper::AltFireCosmetics()
{
#if !_SERVER
	GetLocalPlayer()->WeaponRenderInfo.SetRecoil(4);
	GetLocalPlayer()->PushCameraFov(2, .2, .2);
	GetLocalPlayer()->ShakeCamera(3, .2, 16);
	GetLocalPlayer()->PushCameraRotation(FVec3(4, 0, 0), .1, .5);
	GetLocalPlayer()->AddConcussion(1);

	KEntity* guy = GetCarryingEntity();
	KEntProp_Controllable* cont = guy->As<KEntProp_Controllable>();
	f32 radius = cont && cont->IsViewedEntity() ? 1024 : 192;
	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(255, 179, 127, 255), radius, .07, .3, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);
#endif
}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Zapper::UpdateBuffers(KStaticMesh<"gunplaceholder", "zappertex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true /* is viewed */ )
		FlagBufferForViewWeapon(entry);

	return KBufferUpdateResult(true, vis);
}
#endif
