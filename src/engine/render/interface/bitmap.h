#pragma once

#if !_SERVER

#include "kfglobal.h"

class KHudBitmap
{
protected:

	f32 LastDrawW = -1, LastDrawH = -1;

public:
	
	virtual ~KHudBitmap();

	virtual f32 GetWidth() = 0;
	virtual f32 GetHeight() = 0;
};

#endif