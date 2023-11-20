#pragma once

#if !_COMPILER

#include "kfglobal.h"
#include "engine/math/vec3.h"
#include "../math/polygon.h"

class KLoadedMapEntity
{
public:

	u32 EntityID = 0;
	GFlt Angle;
	GFlt MaxBoundsDimension;
	GVec3 MapPosition;
	bool bIsBrushEntity = false;
	bool bEmitsSound = false;

	TVector<GPolygon> Polygons;
	TVector<GPlane> Planes;

	KLoadedMapEntity() = default;
	KLoadedMapEntity(u32 id, GFlt angle, const GVec3& pos) : EntityID(id), Angle(angle), MapPosition(pos) {}
	
	static UPtr<KLoadedMapEntity> CreateFromFile(class KMapEntity* ent);

	u32 GetID() const { return EntityID; }
	GVec3 GetMapPosition() const { return MapPosition; }
	GFlt GetAngle() const { return Angle; }

	virtual void LinkTarget() {}
	virtual void OnCollisionBrushAssociation() {}
	virtual void UpdateSoundPosition(const GVec3& pos) {}
};

#endif