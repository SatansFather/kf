#pragma once

// wrap in struct for easy bit manipulation without casting
struct ECollisionMask
{
	enum Channels
	{
		None				= 0,
		WorldStatic			= 0x1,
		Light				= 0x2,
		Precipitation		= 0x4,
		Portal				= 0x8,
		Launcher			= 0x10,
		Water				= 0x20,
		Damage				= 0x40,

		// bbox entities
		PlayerCharacter		= 0x10000,
		MonsterCharacter	= 0x20000,
		Gib					= 0x40000,
		Pickup				= 0x80000,
		Weapon				= 0x100000,
		Rubble              = 0x200000,
	};
};
