#pragma once

#if !_SERVER && !_COMPILER

#include "engine/render/fonts/red_october.h"
#include "fonts.h"

static TMap<std::string, KFontData> FontMap = 
{
	{ "Red October", { RedOctober_ttf, RedOctober_ttf_len } },
	{ "Red October Light", { RedOctober_Light_ttf, RedOctober_Light_ttf_len } },
};

KFontData KFontReader::GetFontData(const KString& fontName)
{
	KFontData out;
	if (FontMap.contains(fontName.Get())) out = FontMap[fontName.Get()];
	return out;
}

#endif