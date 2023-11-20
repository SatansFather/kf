#pragma once

#include "engine/global/types_numeric.h"

enum class EInputAction : u8
{
	// state keys must be in this position and in this order
	STATE_START,
	MoveForward,
	MoveBack,
	MoveLeft,
	MoveRight,
	Jump,
	Crouch,
	Fire,
	AltFire,
	STATE_END,

	WEAPON_START,
	PrevWeapon,
	NextWeapon,
	Weapon0,
	Weapon1,
	Weapon2,
	Weapon3,
	Weapon4,
	Weapon5,
	Weapon6,
	Weapon7,
	WEAPON_END,
	
	INTERFACE_START,
	OpenChat,
	LeftClick,
	UpArrow,
	DownArrow,
	LeftArrow,
	RightArrow,
	Submit,
	Pause,
	ShowConsole,
	ShowScoreboard,
	INTERFACE_END,

	INVALID
};

struct EInputKeyState
{
	enum States
	{
		// the order of these is important
		// needs to be the same as the bindings above
		MoveForward = 1,
		MoveBack = 2,
		MoveLeft = 4,
		MoveRight = 8,
		Jump = 16,
		Crouching = 32,
		Firing = 64,
		AltFiring = 128
	};
};