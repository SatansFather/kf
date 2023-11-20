#include "wep_cannon.h"
#include "../properties/controllable.h"
#include "../properties/powerup_inventory.h"
#include "../graphics/flash.h"
#include "engine/game/local_player.h"
#include "engine/audio/audio.h"
#include "engine/net/state.h"
#include "../../../engine/game_instance.h"
#include "../projectiles/proj_cannon.h"
#include "../explosion.h"
#include "../../../engine/net/player.h"

KEntity_Weapon_Cannon::KEntity_Weapon_Cannon()
{
	WeaponID = EWeaponID::Cannon;
	PrimaryFireCooldown = KTime::FramesFromTime(.55);
	AltFireCooldown = KTime::FramesFromTime(.1);
	CooldownSharing = ECooldownSharing::PrimaryRequireAlt;
}

bool KEntity_Weapon_Cannon::Fire()
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
	params.Speed = 850;
	params.FiringWeapon = this;
	params.OwningPlayerIndex = OwningPlayerIndex;
	params.bPrimaryFire = true;
#if !_SERVER
	params.RenderOffset = GVec3(0, 0, -12).GetRotated(pitch, right);
#endif

	KEntity_Projectile_Cannon* proj = KEntity_Projectile_Cannon::Create(params).Get();
	proj->DirectDamage = 25;
	proj->SplashRadius = 150;
	Projectiles.push_back(proj);

	ConsumeAmmo();

	return true;
}

bool KEntity_Weapon_Cannon::AltFire()
{
	if (Projectiles.size() > 0)
	{
		u32 destroyFrame = KTime::FrameCount() + 1;

		for (i32 i = Projectiles.size() - 1; i >= 0; i--)
		{
			if (KEntity_Projectile_Cannon* proj = Projectiles[i].Get())
			{
				if (proj->FiringWeapon.Get() == this)
				{
					u32 minFrame = proj->GetFrameCreated() + 20;
					proj->DestroyFrame = destroyFrame < minFrame ? minFrame : destroyFrame;
					destroyFrame += 4;
				}
			}
		}
		Projectiles.clear();
		return true;
	}

	return false;
}

void KEntity_Weapon_Cannon::PrimaryFireCosmetics()
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

	auto flash = KEntity_LightFlash::CreateFlash(guy->GetPosition(), FColor8(120, 100, 16, 255), radius, .2, .5, 1).Get();
	flash->PositionEntity = this;
	//KAudio::PlaySound(nullptr, .25);

	if (guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Cannon_Primary, 1);
	}
#endif
	if (IsNetServer() && guy)
	{
		KSoundProperties props;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Cannon_Primary, guy->GetPosition(), props);
	}
}

void KEntity_Weapon_Cannon::AltFireCosmetics()
{
	KEntity* guy = GetCarryingEntity();
#if !_SERVER
	if (guy && GetControlledEntity() == guy)
	{
		KAudio::PlaySound(KSoundID::Fire_Cannon_Alt, 1);
	}
#endif
	if (IsNetServer() && guy)
	{
		KSoundProperties props;
		props.ReplicationMethod = EReplicatedSound::SkipIndex;
		props.PlayerIndex = guy->GetOwningPlayer()->OwningPlayerIndex;
		props.bRepOnly = GetControlledEntity() == guy;
		KAudio::PlaySound3D(KSoundID::Fire_Cannon_Alt, guy->GetPosition(), props);
	}
}

#if !_SERVER
KBufferUpdateResult KEntity_Weapon_Cannon::UpdateBuffers(KStaticMesh<"gunplaceholder", "cannontex">& entry)
{
	bool vis = UpdateWeaponRenderBuffer(&entry);
	if (vis && true /* is viewed */) FlagBufferForViewWeapon(entry);
	return KBufferUpdateResult(true, vis);
}
#endif
