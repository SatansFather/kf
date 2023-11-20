#pragma once

#include "kfglobal.h"
#include "../hit_result.h"
#include "../../math/plane.h"

struct KTraceHitParams
{
	GHitResult* Prev = nullptr;
	GHitResult* Test = nullptr;
};

struct KMoveTraceLocalCapture
{
	class KEntProp_Movable* Movable = nullptr;
	class KEntProp_CollidableBBox* Collidable = nullptr;
	TVector<TObjRef<class KEntity_PickupBase>>* PendingItems;
	bool* bInWater = nullptr;
	GHitResult* WaterHit = nullptr;
	u8* Resoves = nullptr;
	KTraceCondition Condition = nullptr;
};

struct KCheckWaterLocalCapture
{
	bool* bInWater = nullptr;
};

struct KPlaneTestLocalCapture
{
	class KEntProp_Movable* Movable = nullptr;
	class KEntProp_CollidableBBox* Collidable = nullptr;
	GPlane* Plane = nullptr;
};

struct KProjectileExplodeLocalCapture
{
	class KEntity_Projectile* Proj = nullptr;
	class KEntProp_CollidableBBox* Collision = nullptr;
	class KEntity* Carrier = nullptr;
	GBoundingBox* ThisBox = nullptr;
	const GVec3* Normal = nullptr;
	const GVec3* PreVel = nullptr;
	const GVec3* Origin = nullptr;
	GFlt Radius = 0;
	GFlt MaxSplashDamage = 0;
	GFlt SplashPushScale = 1;
	GFlt SelfDamageScale = .5;
	bool bHitCarrier = false;
};

struct KCheckPickupLocalCapture
{
	class KEntity_PickupBase* Pickup = nullptr;
};

struct KCompTraceLocalCapture
{
	const class KEntProp_CollidableBBox* ThisBox = nullptr;
};

struct KTelefragLocalCapture
{
	class KEntProp_CollidableBBox* ThisBox = nullptr;
	class KEntity* ThisEnt = nullptr;
};