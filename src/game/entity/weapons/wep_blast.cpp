#include "wep_blast.h"
#include "../properties/controllable.h"
#include "../properties/powerup_inventory.h"
#include "../graphics/flash.h"
#include "engine/game/local_player.h"
#include "engine/audio/audio.h"
#include "engine/net/state.h"
#include "../../../engine/game_instance.h"
#include "../projectiles/proj_blast.h"
#include "engine/net/player.h"

KEntity_Weapon_Blast::KEntity_Weapon_Blast()
{
	WeaponID = EWeaponID::Blast;
	PrimaryFireCooldown = 5;
	AltFireCooldown = 5;
}

KEntity_Weapon_Blast::~KEntity_Weapon_Blast() {}

void KEntity_Weapon_Blast::Tick()
{
	AddCharge(-GameFrameDelta());

	if (IsNetClient() && GetLocalNetPlayer() && OwningPlayerIndex != GetLocalNetPlayer()->OwningPlayerIndex)
	{
		if (bFrenzySound)
		{
			if (!KAudio::SoundIsValid(FrenzyLoop))
			{
				KSoundProperties props;
				props.bLooping = true;
				props.Volume = 1.5;
				FrenzyLoop = KAudio::PlaySoundAttached(KSoundID::Blaster_Frenzy_Loop, GetCarryingEntity(), props);
				//KAudio::FadeVolume(FrenzyLoop, 1, .1);
			}
		}
		else if (KAudio::SoundIsValid(FrenzyLoop))
		{
			//	KAudio::FadeVolume(FrenzyLoop, 1, 0);
			KAudio::StopSound(FrenzyLoop);
		}
	}
	else
	{
		if (HasFrenzyPowerup())
		{
			if (GetCooldownState() == EWeaponCooldown::Primary || GetCooldownState() == EWeaponCooldown::Alt)
			{
				if (!KAudio::SoundIsValid(FrenzyLoop))
				{
					KSoundProperties props;
					props.bLooping = true;
					props.Volume = 1.5;
					FrenzyLoop = KAudio::PlaySoundAttached(KSoundID::Blaster_Frenzy_Loop, GetCarryingEntity(), props);
					//KAudio::FadeVolume(FrenzyLoop, 1, .1);
				}
				bFrenzySound = true;
			}
			else 
			{
				//	KAudio::FadeVolume(FrenzyLoop, 1, 0);
				if (KAudio::SoundIsValid(FrenzyLoop))
					KAudio::StopSound(FrenzyLoop);
				bFrenzySound = false;
			}
		}
		else
		{
			if (KAudio::SoundIsValid(FrenzyLoop))
			{
			//	KAudio::FadeVolume(FrenzyLoop, 1, 0);
				KAudio::StopSound(FrenzyLoop);
			}
			bFrenzySound = false;
		}
	}
}

void KEntity_Weapon_Blast::Poolable_PreDestroy()
{
	KEntity::Poolable_PreDestroy();

	if (KAudio::SoundIsValid(FrenzyLoop))
		KAudio::StopSound(FrenzyLoop);
}

void KEntity_Weapon_Blast::AddCharge(GFlt charge)
{
	Charge += charge;
	Charge = KSaturate(Charge);
}

bool KEntity_Weapon_Blast::HasFrenzyPowerup()
{
	KEntity* ent = GetCarryingEntity();
	if (!ent) return false;

	if (KEntProp_PowerupInventory* inv = ent->As<KEntProp_PowerupInventory>())
		return inv->HasPowerup(EPowerupID::Brain);

	return false;
}

bool KEntity_Weapon_Blast::Fire()
{	
	KFireInputInfo info;
	GetInputInfo(info);

	KProjectileCreationParams params;
	params.Position = info.CamPosition;
	params.Direction = info.Forward;
	params.Speed = 2000;
	params.FiringWeapon = this;
	params.OwningPlayerIndex = OwningPlayerIndex;
	params.bPrimaryFire = false;
#if !_SERVER
	params.RenderOffset = GVec3(0, 0, -12).GetRotated(info.Pitch, info.Right);
	params.RenderOffset += info.Forward * 24;
#endif

	KEntity_Projectile_Blast* proj = KEntity_Projectile_Blast::Create(params).Get();
	proj->DirectDamage = 0;

	ConsumeAmmo();

	return true;
}

bool KEntity_Weapon_Blast::AltFire()
{
	return Fire();

	AddCharge(GameFrameDelta() * 2);

	if (Charge >= 1)
	{
		Charge = 0;

		KFireInputInfo info;
		GetInputInfo(info);

		KProjectileCreationParams params;
		params.Position = info.CamPosition;

		params.Direction = info.Forward;
		params.Speed = 2000;
		params.FiringWeapon = this;
		params.OwningPlayerIndex = OwningPlayerIndex;
		params.bPrimaryFire = false;
#if !_SERVER
		params.RenderOffset = GVec3(0, 0, -12).GetRotated(info.Pitch, info.Right);
#endif

		for (i32 i = -3; i <= 3; i++)
		{
			params.Direction = info.Forward + (info.Right * .1 * i);
			KEntity_Projectile_Blast* proj = KEntity_Projectile_Blast::Create(params).Get();
		}

		return true;
	}

	return false;

	return true;
}

void KEntity_Weapon_Blast::PrimaryFireCosmetics()
{
	KEntity* guy = GetCarryingEntity();
	bool frenzy = HasFrenzyPowerup();
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
			GetLocalPlayer()->WeaponRenderInfo.SetRecoil(1);
			GetLocalPlayer()->PushCameraFov(.7, .1, .4);
			GetLocalPlayer()->ShakeCamera(.5, .2, 24);
			GetLocalPlayer()->PushCameraRotation(FVec3(.5, 0, 0), .15, .35);
			GetLocalPlayer()->AddConcussion(.1);
		}
	}
	f32 radius = cont && cont->IsViewedEntity() ? 1024 * KGameInstance::Get().bWeaponFlash : 192;

	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(20, 20, 100, 255), radius, .08, .25, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);
	if (!frenzy && guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Blaster_Primary, .7);
	}
#endif
	if (!frenzy && IsNetServer() && guy)
	{
		KSoundProperties props;
		props.Volume = .7;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Blaster_Primary, guy->GetPosition(), props);
	}
}

void KEntity_Weapon_Blast::AltFireCosmetics()
{
	PrimaryFireCosmetics();
}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Blast::UpdateBuffers(KStaticMesh<"gunplaceholder", "blastertex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true /* is viewed */) FlagBufferForViewWeapon(entry);
	return KBufferUpdateResult(true, vis);
}
#endif
