#pragma once

#include "kfglobal.h"

typedef std::array<u8, 16> KMapHash;

// represents a map that has been processed and is known to the application
struct KLocalMap
{
	KMapHash Hash;
	KString MapFileName;
	KString MapIngameName;
	KString WadName = ""; // empty wad name means raw .kmap file
};