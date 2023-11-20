#pragma once

#if !_SERVER && !_COMPILER

#include "kfglobal.h"

struct KFontData
{
	u8* Data = nullptr;
	u32 Size = 0;
};

struct KFontReader
{
	static KFontData GetFontData(const KString& fontName);
};

#endif