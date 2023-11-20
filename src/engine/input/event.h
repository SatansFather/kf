#pragma once

#include "binding.h"

class KInputKeyEvent
{
private:

	EInputAction Event = EInputAction::INVALID;
	i32 Input = 0;
	i32 Mods = 0;

public:

	KInputKeyEvent(EInputAction evnt, i32 input, i32 mods) : Event(evnt), Input(input), Mods(mods) {}

	inline EInputAction GetEvent() const { return Event; }
	inline i32 GetInput() const { return Input; }
	inline i32 GetMods() const { return Mods; }
};

class KInputMouseEvent
{
private:
	
	f64 x = 0;
	f64 y = 0;

public:

	KInputMouseEvent(f64 x, f64 y) : x(x), y(y) {}

	inline f64 GetX() { return x; }
	inline f64 GetY() { return y; }

};

enum class EMouseMoveType
{
	Move,
	Set,
	Show
};
