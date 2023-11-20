#pragma once

#include "engine/global/types_numeric.h"
#include "engine/math/math.h"

struct EPowerupID
{
	enum 
	{
		Brain = 0,
		Invis = 1,
		NumPowerups
	};

	//static constexpr u32 GetPowerupCount()
	//{
	//	return u32(cilog2(NumPowerups)) + 1;
	//}
};	