#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_BloodTrail : public KEntity, 
public KEntProp_Renderable<KBloodTrail>
{
	f32 TimeCreated;

public:

	static void Create(const GVec3& position);
	
	KEntity_BloodTrail();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KBloodTrail& entry) override;
#endif

};

class KEntity_BloodTrail_UnderWater : public KEntity,
	public KEntProp_Renderable<KBloodTrail_UnderWater>
{
	f32 TimeCreated;

public:

	static void Create(const GVec3& position);

	KEntity_BloodTrail_UnderWater();

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KBloodTrail_UnderWater& entry) override;
#endif

};

#endif