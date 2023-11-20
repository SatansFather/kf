#pragma once

#include "kfglobal.h"
#include "engine/math/vec3.h"

class KSpecPoint
{
public:
	GVec3 Position;
	GVec3 Rotation;
	KSpecPoint* Next = nullptr;
};