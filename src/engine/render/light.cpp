#if !_SERVER

#include "light.h"
#include "interface/render_interface.h"
#include "interface/render_target.h"
#include "interface/buffers/gpu_buffer.h"
#include "interface/rasterizer.h"
#include "scene.h"

// DELETE
#include "../../compiler/compiler.h"

KLight::KLight(u32 res) 
{
	SetDepthResolution(res);
}

KLight::~KLight() {}

void KLight::UpdateLightDepthBuffer()
{
	if (KRenderInterface* iface = GetRenderInterface())
	{
		// position data
		BuildLightMatrix();
		glm::mat4 model(1.f); // identity matrix for static world
		glm::mat4 mvplight = glm::transpose(LightMatrix * model);
		iface->SetModelTransform(model);
		iface->SetModelViewProjection(mvplight);

		// views
		// null render target disables color writes, much faster
		DepthBuffer->Clear();
		iface->BindRenderTarget(nullptr, DepthBuffer.get());
		iface->SetViewport(DepthRes, DepthRes);

		// scene rendering
		iface->UpdateBuffer(iface->LightMatrixBuffer.get(), &LightMatrix);
		iface->BindTexture2D(iface->GetWorldScene()->GetTextureAtlas(), 0);
		iface->GetRasterizer()->SetCullMode(ECullMode::CullBack);
		iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
		iface->BindShaderProgram(iface->Shaders.LightDepth.get());

		// check it out
		iface->GetWorldScene()->DrawOpaque();
	}
}

void KLight::SetDepthResolution(f32 res)
{
	DepthRes = res;

	//RenderTarget.reset();
	DepthBuffer.reset();

	DepthBuffer = GetRenderInterface()->CreateDepthBuffer(EDepthUsage::ShadowDepth, DepthRes, DepthRes);
	DepthBuffer->SetEnabled(true);
	DepthBuffer->SetReadOnly(false);

	//FRenderTargetCreationFlags f;
	//RenderTarget = GetRenderInterface()->CreateRenderTarget(DepthRes, DepthRes, f);
}

#endif
