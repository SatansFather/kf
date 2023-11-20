#if !_COMPILER

// this function gets its own document because it needs to include a lot

#include "game_instance.h"

#include "game/entity/properties/renderable.h"
#include "game/entity/character_player.h"
#include "game/entity/spectator.h"
#include "game/entity/death_cam.h"
#include "game/entity/character_monster.h"
#include "game/entity/graphics/shotgun_trail.h"
#include "game/entity/graphics/hit_spark.h"
#include "game/entity/graphics/flash.h"
#include "game/entity/weapons/wep_shotgun.h"
#include "game/entity/graphics/water_splash.h"
#include "game/entity/graphics/bullet_hole.h"
#include "game/entity/graphics/gibs.h"
#include "game/entity/graphics/blood_trail.h"
#include "game/entity/graphics/torch.h"
#include "game/entity/pickups/powerups/pickup_powerup_brain.h"
#include "game/entity/projectiles/proj_shotgun.h"
#include "game/entity/projectiles/proj_atom.h"
#include "game/entity/powerups/powerup_brain.h"
#include "game/entity/weapons/wep_zapper.h"
#include "game/entity/weapons/wep_rocket.h"
#include "game/entity/projectiles/proj_rocket.h"
#include "game/entity/explosion.h"
#include "game/entity/map/projectile_launcher.h"
#include "game/entity/graphics/rocket_trail.h"
#include "game/entity/pickups/pickup_health.h"
#include "game/entity/graphics/portal_travel.h"
#include "game/entity/graphics/lightning_bolt.h"
#include "../game/entity/powerups/powerup_invis.h"
#include "../game/entity/pickups/powerups/pickup_powerup_invis.h"
#include "../game/entity/weapons/wep_cannon.h"
#include "../game/entity/weapons/wep_blast.h"
#include "../game/entity/projectiles/proj_cannon.h"
#include "../game/entity/projectiles/proj_shotgun_boulder.h"
#include "../game/entity/graphics/rubble.h"
#include "../game/entity/pickups/pickup_weapon.h"
#include "../game/entity/smoke_sheet.h"
#include "../game/entity/projectiles/proj_blast.h"
#include "../game/entity/blast_explosion.h"

u32 TotalSize = 0;

template <typename T>
KDataPoolBase* AddPool(u32 count, bool ignoreEntity = false, KPool::EOverflowResponse overflowResponse = KPool::EOverflowResponse::Overflow)
{
	auto pool = TDataPool<T>::CreatePool(count, 0, overflowResponse);
#if !_SERVER
	if constexpr (std::is_base_of<KRenderablePropBase, T>::value)
		RenderableDataPools.push_back(pool);
#endif
	if (std::is_base_of<KEntity, T>::value && !ignoreEntity)
		EntityDataPools.push_back(pool);
	if (std::is_base_of<KSnapshottable, T>::value)
		SnapshotDataPools.push_back(pool);
	if (std::is_base_of<KSnapshottable, T>::value && std::is_base_of<KEntProp_CollidableBBox, T>::value)
		HistoryDataPools.push_back(pool);

#if !_SERVER
	T t;
	if (KRenderablePropBase* p = dynamic_cast<KRenderablePropBase*>(&t))
		p->AllocateRenderBuffer(count);
#endif

	TotalSize += sizeof(T) * count;

	return TDataPool<T>::GetPool();
}

void KGameInstance::CreateMemPools()
{
	KTimePoint start = KTime::Now();

#if !_SERVER
	AddPool<KEntity_WallTorch>(128);
#endif

	AddPool<KEntity_BlastExplosion>(12, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_Explosion>(12, true, KPool::EOverflowResponse::ReplaceOld);

	AddPool<KEntity_Projectile_ShotgunBoulder>(1024); 
	AddPool<KEntity_Projectile_ShotgunShard>(4096); 
	AddPool<KEntity_Projectile_Blast>(2048); 
	AddPool<KEntity_Projectile_Rocket>(1024);
	AddPool<KEntity_Projectile_Cannon>(1024);
	AddPool<KEntity_Projectile_Atom>(64);
	AddPool<KEntity_Character_Player>(256);
	AddPool<KEntity_Character_Monster>(4096);
	AddPool<KEntity_Powerup_Brain>(16, false, KPool::EOverflowResponse::PreventNew);
	AddPool<KEntity_Powerup_Invis>(16, false, KPool::EOverflowResponse::PreventNew);
	AddPool<KEntity_Pickup_Powerup_Brain>(16, false, KPool::EOverflowResponse::PreventNew);
	AddPool<KEntity_Pickup_Powerup_Invis>(16, false, KPool::EOverflowResponse::PreventNew);

	AddPool<KEntity_Pickup_Health>(128);
	AddPool<KEntity_Pickup_Weapon_Rocket>(32);
	AddPool<KEntity_Pickup_Weapon_Cannon>(32);
	AddPool<KEntity_Pickup_Weapon_Shotgun>(32);
	AddPool<KEntity_Pickup_Weapon_Blast>(32);

	// TODO overflow in mempool is somewhat broken
	// resizing vector calls constructor
	// any new allocation calls constructor

	AddPool<KEntity_Weapon_Shotgun>(32);
	AddPool<KEntity_Weapon_Rocket>(32);
	AddPool<KEntity_Weapon_Zapper>(32);
	AddPool<KEntity_Weapon_Cannon>(32);
	AddPool<KEntity_Weapon_Blast>(32);

	AddPool<KEntity_ProjectileLauncher>(12);


#if !_SERVER
	AddPool<KEntity_Spectator>(1, false, KPool::EOverflowResponse::PreventNew);
	AddPool<KEntity_DeathCamera>(1);

	AddPool<KEntity_Gib_MeatChunk>(1024, false, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_Rubble>(512, false, KPool::EOverflowResponse::ReplaceOld);

	//AddPool<KEntity_SmokeSheet>(128, KPool::EOverflowResponse::ReplaceOld);

	AddPool<KEntity_LightFlash>(32, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_HitSpark>(512, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_WaterSplash>(512, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_PortalTravel>(32, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_LightningBolt>(32, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_BulletHole>(512, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_BloodTrail>(8192, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_BloodTrail_UnderWater>(8192, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_ShotgunTrail>(512, true, KPool::EOverflowResponse::ReplaceOld);
	AddPool<KEntity_RocketTrail>(512, true, KPool::EOverflowResponse::ReplaceOld);

	GetRenderInterface()->bMemPoolsCreated = true;

#endif

	f64 loadTime = KTime::Since(start);
	LOG("Allocated " + KString(f64(TotalSize) / 1024.0 / 1024.0, 2) + " MB for entity data in " + 
		KString(loadTime, 2) + (loadTime == 1 ? " second" : " seconds"));
}

#endif