#pragma once

#include "../../entity.h"

class KEntity_FireEvent : public KEntity
{
	bool bIsLocal = false;
	GVec3 Direction;
};