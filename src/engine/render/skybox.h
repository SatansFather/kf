#pragma once

#if !_SERVER

#include "kfglobal.h"

class KSkybox
{
	glm::vec3 CameraPosition;

	UPtr<class KVertexBuffer> StaticVertexBuffer;
	u32 StaticVertexCount;
	u32 StaticIndexCount;

	UPtr<class KTexture2D> TextureAtlas;
	UPtr<class KTexture2D> LightmapTexture;
};

#endif