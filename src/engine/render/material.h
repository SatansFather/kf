#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "interface/rasterizer.h"

class KMaterial
{
public:

	KString MaterialName = "";
	class KShaderProgram* Shader = nullptr;
	TVector<class KTexture2D*> VertexTextures;
	TVector<class KTexture2D*> PixelTextures;
	ECullMode CullMode = ECullMode::CullBack;
	bool bTransparent = false; // uses transparent blend state
	bool bWriteDepth = true;
	bool bAppendMainRenderTarget = true; // binds post target
	bool bTransparentPass = false; // default blend state, composited onto post target
	
	void Bind();
	std::function<void()> ExtraBindFunction;
};

#endif