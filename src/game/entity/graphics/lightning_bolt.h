#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"
#include "engine/math/line_segment.h"

struct KBoltSegment
{
	GLineSegment Line;
	f32 Thickness;
};

class KEntity_LightningBolt : public KEntity/*, 
public KEntProp_Renderable<KLightningBolt>*/
{


public:
	
	KEntity_LightningBolt();
	static void Create(const GVec3& start, const GVec3& end, f32 thickness = 1);
	/*

	KBufferUpdateResult UpdateBuffers(KLightningBolt& entry) override;*/
};

#endif