	#pragma once

#include "kfglobal.h"
#include "binding.h"

struct KKeyNames
{
	static KString GetKeyName(i32 key);
	static i32 GetKeyFromName(const KString& name);
	static KString GetActionName(u8 bind);
	static KString GetActionName(EInputAction bind);
	static u8 GetActionFromName(const KString& name);
	static KString CorrectActionCase(const KString& action);
	static bool KeyNameIsValid(const KString& k);
};