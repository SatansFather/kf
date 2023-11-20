#pragma once

#include "engine/math/vec3.h"

template <typename T>
struct KHitResult
{
	enum EObjectType
	{
		None        = 0b00000000,
		WorldBrush  = 0b00000001,
		BoxEnt      = 0b00000010,
	};

	enum EHitFlags
	{
		StartSolid	= 0b10000000,
		AllSolid	= 0b01000000,
		Block		= 0b00100000,
	};

	KVec3<T> Point;
	KVec3<T> Normal;
	void* Object = nullptr;
	const class KEntProp_CollidableBBox* BoxEntity = nullptr;
	T PlaneDistance = 0;
	T Time = 1;
	T PenetrationDepth = 0;
	T PositionOffsetZ = 0;
	u32 TraceCollision = 0;		// collision type of the trace
	u32 SearchCollision = 0;	// collision type trace is looking for
	u32 HitCollision = 0;		// collision type of hit object
	u8 bHit = None; // this stores the object type but can be treated like a bool

	template <typename Type>
	KHitResult<Type> ToType()
	{
		KVec3<Type> p(Point.x, Point.y, Point.z);
		KVec3<Type> n(Normal.x, Normal.y, Normal.z);
		Type t = Time;
		KHitResult<Type> h;
		h.Point = p;
		h.Normal = n;
		h.Time = t;
		h.bHit = bHit;
		h.PlaneDistance = 0;
		h.Object = Object;
		h.TraceCollision = TraceCollision;
		h.HitCollision = HitCollision;
		h.SearchCollision = SearchCollision;
		h.PenetrationDepth = PenetrationDepth;
		h.BoxEntity = BoxEntity;
		return h;
	}

	bool IsAllSolid() const { return bHit & AllSolid; }
	bool StartsSolid() const { return bHit & StartSolid; }
	bool HitBoxEntity() const { return bHit & BoxEnt; }
	bool HitWorldBrush() const { return bHit & WorldBrush; }
	void Reset();

#if !_COMPILER
	class KLoadedMapEntity* GetMapEntity();
#endif

	operator bool() const { return bHit > 0; }
};

typedef KHitResult<f64> DHitResult;
typedef KHitResult<f32> FHitResult;
typedef KHitResult<GFlt> GHitResult;

typedef bool(*KTraceCondition)(const GHitResult&, const GHitResult&);