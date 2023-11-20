#pragma once

#include "kfglobal.h"
#include "sound_asset.h"

struct KSoundID
{
	enum
	{
		Null_Sound,
		
		// player movement
		Player_Jump,
		Player_Footstep_Generic,
		Player_Land_Generic,
		Player_Wall_Bump,

		// player exertions
		Player_Pain_1,
		Player_Pain_2,
		Player_Pain_3,
		Player_Pain_4,
		Player_Death_1,
		Player_Death_2,

		Player_Step_1,
		Player_Step_2,
		Player_Step_3,

		Player_Spawn,

		// weapon fire
		Fire_Rocket_Primary,
		Fire_Rocket_Alt,
		Fire_Shotgun_Primary,
		Fire_Shotgun_Alt,
		Fire_Cannon_Primary,
		Fire_Cannon_Alt,
		Fire_Blaster_Primary,
		Fire_Blaster_Alt,

		Blaster_Frenzy_Loop,

		// projectile hum
		Proj_Rocket_Hum,

		Rocket_Explode_1,
		Rocket_Explode_2,

		Cannon_Explode_1,

		Shard_Hit_1,
		Shard_Hit_2,
		Shard_Hit_3,
		Shard_Hit_4,

		// entity ambient
		Ambient_Portal,
		Ambient_Launcher,

		Invis_On,
		Invis_Off,

		// entity effects
		Enter_Portal,
		Launcher_Launch,

		Pickup_Gun,
		Respawn_Weapon,

		Health_Respawn,

		Respawn_Brain,
		Despawn_Brain,
		Ambient_Brain,
		Carry_Brain,
		Drop_Brain,

		Despawn_HP_Small,
		Despawn_HP_Med,
		Despawn_HP_Big,
		Respawn_HP_Small,
		Respawn_HP_Med,
		Respawn_HP_Big,

		// world ambient
		Ambient_Underwater_Shallow,
		Ambient_Underwater_Deep,

		// interface
		Menu_Close,
		Menu_Open,
		Menu_Scroll,

		Chat_Beep,

		Spree_Announce,
		Spree_Ended,

		Score_Frag,
		Score_Death,

		Damage_Tick_0,
		Damage_Tick_1,
		Damage_Tick_2,
		Damage_Tick_3,
		Damage_Tick_4,
		Damage_Tick_5,
		Damage_Tick_6,
		Damage_Tick_7,
		Damage_Tick_8,
		Damage_Tick_9,
		Damage_Tick_10,
		Damage_Tick_11,

		NumSounds
	};

	typedef u8 SoundInt;

	SoundInt Index = NumSounds;

	static TVector<UPtr<KSoundAsset>> SoundList;
	static void LoadSounds(); // found in sound_load.cpp which is python generated

	KSoundID() = default;
	KSoundID(SoundInt index) : Index(index) {}
};

// id is KSoundID::SoundName
class KSoundAsset* GetSoundAsset(KSoundID id);