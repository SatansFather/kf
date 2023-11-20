#if !_SERVER

#include "material.h"
#include "interface/render_interface.h"

void KMaterial::Bind()
{
	if (KRenderInterface* iface = GetRenderInterface())
	{
#if !_COMPILER
		//if (bAppendMainRenderTarget)
		//	iface->BindRenderTarget(iface->SolidTarget.get(), iface->SceneDepthBuffer.get());
#endif

		if (ExtraBindFunction) ExtraBindFunction();
	
		iface->GetRasterizer()->SetCullMode(CullMode);
		bTransparent ? iface->EnableTransparency() : iface->DisableTransparency();
		iface->SetDepthBufferReadOnly(!bWriteDepth);

		for (u32 i = 0; i < VertexTextures.size(); i++)
			iface->BindTexture2D(VertexTextures[i], i, EShaderStage::Vertex);

		for (u32 i = 0; i < PixelTextures.size(); i++)
			iface->BindTexture2D(PixelTextures[i], i, EShaderStage::Pixel);
		
		iface->BindShaderProgram(Shader);
		if (Shader == GetRenderInterface()->Shaders.WorldWireframe.get())
			iface->GetRasterizer()->SetFillMode(EFillMode::Wireframe);
		else
			iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	}
}

#endif

