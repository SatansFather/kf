#pragma once

#if !_SERVER

#include "../entity.h"
#include "../properties/renderable.h"

class KEntity_WallTorch : public KEntity, 
	public KEntProp_Renderable<KStaticMesh<"torch1", "kf/metal2">, KTorchFlame>
{

public:

	f32 Yaw;
	FColor8 MainColor, OffColor;

#if !_SERVER
	KBufferUpdateResult UpdateBuffers(KStaticMesh<"torch1", "kf/metal2">& mesh, KTorchFlame& flame) override;
#endif

};

#endif