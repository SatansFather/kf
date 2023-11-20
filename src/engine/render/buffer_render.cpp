#if !_SERVER

#include "bufferslots.h"
#include "interface/buffers/gpu_buffer.h"
#include "engine/render/interface/render_interface.h"
#include "interface/rasterizer.h"
#include "particle.h"
#include "surface2d.h"
#include "interface/texture2d.h"
#include "../../game/entity/properties/renderable.h"
#include "interface/render_target.h"

UPtr<KTexture2D> KSmokeSheet::Texture;
UPtr<KTexture2D> KSmokeBeam::Tex1, KSmokeBeam::Tex2;
UPtr<KGpuBuffer> KSmokeBeam::IndexMapBuffer;
TVector<u32> KSmokeBeam::CpuIndexMap;
TVector<u32> KSmokeBeam::GpuIndexMap;
u32 KSmokeBeam::CpuParticleCount;
u32 KSmokeBeam::BridgeParticleCount;
u32 KSmokeBeam::GpuParticleCount;

UPtr<class KTexture2D> KJohnnyJiantdick::JohnnyPic;

UPtr<KTexture2D> KBulletHole::Texture;

KModelObj* KPowerupBrain::Model;
UPtr<KTexture2D> KPowerupBrain::Texture;

KModelObj* KHealthCrystal::Model;
UPtr<KTexture2D> KHealthCrystal::Tex1;
UPtr<KTexture2D> KHealthCrystal::Tex2;

void KPortalData::InitFromMap(TMap<KString, KString>& m)
{
	// init to defaults
	SetBackColor(FColor32(.6, 0, .6, 1).To8());
	SetRippleColor(FColor32(.2, .05, .1, 1).To8());
	SetHeightRatio(1);
	SetRippleRate(1.5);
	SetTimeScale(4);

	if (m.contains("anim_rate"))
	{
		f32 val;
		if (m["anim_rate"].ToFloatSafe(val))
			SetTimeScale(val);
	}
	if (m.contains("ripple_density"))
	{
		f32 val;
		if (m["ripple_density"].ToFloatSafe(val))
			SetRippleRate(val);
	}
	if (m.contains("height_ratio"))
	{
		f32 val;
		if (m["height_ratio"].ToFloatSafe(val))
			SetHeightRatio(val);
	}
	if (m.contains("back_color"))
	{
		TVector<f32> val = m["back_color"].ToFloatArray();
		if (val.size() == 3)
			SetBackColor(FColor32(val[0], val[1], val[2], 1).To8());
	}
	if (m.contains("ripple_color"))
	{
		TVector<f32> val = m["ripple_color"].ToFloatArray();
		if (val.size() == 3)
			SetRippleColor(FColor32(val[0], val[1], val[2], 1).To8());
	}
}

void KWaterData::InitFromMap(TMap<KString, KString>& m)
{
	// init to defaults
	SetMaxDepth(128);
	SetWaterizeScale(1);
	SetWaterColor(FColor32(.35, .25, .23, .64).To8());
	SetCaustic1(FColor32(.5, .5, .2, 1).To8());
	SetCaustic2(FColor32(.25, .25, .25, .5).To8());

	if (m.contains("max_depth"))
	{
		f32 val;
		if (m["max_depth"].ToFloatSafe(val))
			SetMaxDepth(val);
	}
	if (m.contains("waterize_scale"))
	{
		f32 val;
		if (m["waterize_scale"].ToFloatSafe(val))
			SetWaterizeScale(val);
	}
	if (m.contains("water_color"))
	{
		TVector<f32> val = m["water_color"].ToFloatArray();
		if (val.size() == 4)
			SetWaterColor(FColor32(val[0], val[1], val[2], 1).To8());
	}
	if (m.contains("caustic_color_1"))
	{
		TVector<f32> val = m["caustic_color_1"].ToFloatArray();
		if (val.size() == 4)
			SetCaustic1(FColor32(val[0], val[1], val[2], 1).To8());
	}
	if (m.contains("caustic_color_2"))
	{
		TVector<f32> val = m["caustic_color_2"].ToFloatArray();
		if (val.size() == 4)
			SetCaustic2(FColor32(val[0], val[1], val[2], 1).To8());
	}
}

void KBoundingBoxRender::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullBack);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.Cube.get());
	iface->DrawIndexed(info.ActiveCount * 36, 0);
}

void KBoundingBoxRender::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);

	// create index buffer only
	u32 ind[] = 
	{
		// cube faces
		0, 2, 1, 2, 3, 1,
		5, 4, 1, 1, 4, 0,
		0, 4, 6, 0, 6, 2,
		6, 5, 7, 6, 4, 5,
		2, 6, 3, 6, 7, 3,
		7, 1, 3, 7, 5, 1
	};

	TVector<u32> indices;
	u32 size = sizeof(ind) / sizeof(ind[0]);
	indices.resize(size * elementCount);
	u32 vertCount = 8;
	for (u32 i = 0; i < elementCount * size; i++)
	{
		u32 index = i % size;
		u32 particle = i / size;
		indices[i] = ind[index] + particle * vertCount;
	}
	info.VertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(nullptr, 0, 0, indices);
}

bool KBoundingBoxRender::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KSmokeBeam::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	//iface->EnableTransparency();
	//iface->BindRenderTarget(iface->TransparencyTarget.get(), iface->SceneDepthBuffer.get());
	//iface->EnableOIT();
	//iface->SetDepthBufferReadOnly(true);
	iface->BindBuffer(info.GpuBuffer.get());
	iface->BindBuffer(IndexMapBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.SmokeBeam.get());
	//iface->WaterDepthTarget->BindTexture2D(2, EShaderStage::Vertex);
	iface->BindTexture2D(Tex1.get(), 2);
	iface->BindTexture2D(Tex2.get(), 3);
	iface->DrawIndexed(GpuParticleCount * 6, 0);
	//iface->DisableTransparency();
	//iface->DisableOIT();
}

void KSmokeBeam::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.bTransparent = true;

	KRenderInterface* iface = GetRenderInterface();
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = iface->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = iface->CreateParticleIndexBuffer(SMOKE_BEAM_MAX_PARTICLES);

	CpuIndexMap.resize(SMOKE_BEAM_MAX_PARTICLES);
	GpuIndexMap.resize(SMOKE_BEAM_MAX_PARTICLES);
	IndexMapBuffer = iface->CreateStructuredBufferDynamic(sizeof(u32), 1, EShaderStage::Vertex, SMOKE_BEAM_MAX_PARTICLES);

	// load textures
	Tex1 = iface->LoadTexture2D("kf/effects/smoke_1");
	Tex2 = iface->LoadTexture2D("kf/effects/smoke_mask_2");

	const auto extraUpdate = [](bool gpu, bool viewWeapon) -> void
	{
		if (gpu)
		{
			KSmokeBeam::GpuParticleCount = KSmokeBeam::BridgeParticleCount;
			if (KSmokeBeam::GpuParticleCount > 0)
				GetRenderInterface()->UpdateBuffer(KSmokeBeam::IndexMapBuffer.get(), KSmokeBeam::GpuIndexMap.data(), KSmokeBeam::GpuParticleCount);
			return;
		}

		if (KSmokeBeam::CpuParticleCount > 0)
		{
			memcpy(KSmokeBeam::GpuIndexMap.data(), KSmokeBeam::CpuIndexMap.data(), KSmokeBeam::CpuParticleCount * 4);
			KSmokeBeam::BridgeParticleCount = KSmokeBeam::CpuParticleCount;
		}
	};

	info.ExtraUpdateFunction = extraUpdate;
}

bool KSmokeBeam::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KHitSpark::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.HitSpark.get());
	iface->DrawIndexed(info.ActiveCount * 6 * HIT_SPARK_COUNT, 0);
}

void KHitSpark::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * HIT_SPARK_COUNT);
}

bool KHitSpark::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KWaterSplash::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.WaterSplash.get());
	iface->DrawIndexed(info.ActiveCount * 6 * WATER_SPLASH_COUNT, 0);
}

void KWaterSplash::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * WATER_SPLASH_COUNT);
	info.bTransparent = true;
}

bool KWaterSplash::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KShotgunShard::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullFront);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.ShotgunShard.get());
	iface->DrawIndexed(info.ActiveCount * 24, 0);
}

void KShotgunShard::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);

	// create index buffer only
	u32 ind[] =
	{
		0, 3, 1,
		0, 2, 3,
		2, 0, 4,
		4, 0, 1,

		5, 4, 1,
		5, 2, 4,
		5, 3, 2,
		5, 1, 3
	};

	TVector<u32> indices;
	u32 size = sizeof(ind) / sizeof(ind[0]);
	indices.resize(size * elementCount);
	u32 vertCount = 6;
	for (u32 i = 0; i < elementCount * size; i++)
	{
		u32 index = i % size;
		u32 particle = i / size;
		indices[i] = ind[index] + particle * vertCount;
	}
	info.VertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(nullptr, 0, 0, indices);
}

bool KShotgunShard::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KLightFlash::RenderBuffer(const KRenderBufferInfo& info)
{
	
}

void KLightFlash::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	KRenderInterface* iface = GetRenderInterface();

	info.GpuBuffer.reset();
	info.GpuBuffer = iface->CreateConstantBufferDynamic(typeSize * MAX_FLASHES, CB_LIGHTFLASH, EShaderStage::All);

	iface->BindBuffer(info.GpuBuffer.get());
}

bool KLightFlash::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KStaticMeshBase::MeshRenderBuffer(const KRenderBufferInfo& info, KModelObj* model, KTexture2D* tex)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindShaderProgram(iface->Shaders.StaticMesh.get());
	if (tex) iface->BindTexture2D(tex, 0);
	iface->BindVertexBuffer(model->GetVertexBuffer(), info.bDrawingViewWeapon ? info.GpuBuffer_ViewWeapon.get() : info.GpuBuffer.get());
	iface->DrawInstanced(model->GetVertexCount(), info.bDrawingViewWeapon ? 1 : info.ActiveCount);

	/*if (!info.bDrawingViewWeapon)
	{
		iface->GetRasterizer()->SetCullMode(ECullMode::CullFront);
		iface->BindShaderProgram(iface->Shaders.StaticMeshScaledOutline.get());
		iface->BindVertexBuffer(model->GetVertexBuffer(), info.bDrawingViewWeapon ? info.GpuBuffer_ViewWeapon.get() : info.GpuBuffer.get());
		iface->DrawInstanced(model->GetVertexCount(), info.bDrawingViewWeapon ? 1 : info.ActiveCount);
	}*/
}

void KStaticMeshBase::MeshCreateBuffers(const char* meshName, const char* texName, UPtr<KModelObj>& model, UPtr<KTexture2D>& texture, 
	KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	if (!model) 
	{
		model = std::make_unique<KModelObj>();
		model->LoadFromFile(KString(meshName));
	}

	info.GpuBuffer = GetRenderInterface()->CreateInstanceBufferDynamic(typeSize, elementCount);

	if (!texture && KString(texName) != "")
		texture = GetRenderInterface()->LoadTexture2D(KString(texName));

	info.GpuBuffer_ViewWeapon = GetRenderInterface()->CreateInstanceBufferDynamic(typeSize, 1);
}

/*
void KStaticMeshBase_ViewWeapon::MeshRenderBuffer(const KRenderBufferInfo& info, class KModelObj* model, KTexture2D* tex)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->GetRasterizer()->SetCullMode(ECullMode::CullFront);
	iface->BindShaderProgram(iface->Shaders.StaticMesh.get());
	if (tex) iface->BindTexture2D(tex, 0);
	iface->BindVertexBuffer(model->GetVertexBuffer(), info.GpuBuffer.get());
	iface->DrawInstanced(model->GetVertexCount(), info.ActiveCount);
}

void KStaticMeshBase_ViewWeapon::MeshCreateBuffers(const char* meshName, const char* texName, class KModelObj*& model, UPtr<KTexture2D>& texture, KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	model = new KModelObj;
	model->LoadFromFile(KString(meshName));
	info.GpuBuffer = GetRenderInterface()->CreateInstanceBufferDynamic(typeSize, elementCount);
	info.bIsViewWeapon = true;
	if (KString(texName) != "")
		texture = GetRenderInterface()->LoadTexture2D(KString(texName));
}*/

void KJohnnyJiantdick::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.DoomSprite.get());
	iface->BindTexture2D(JohnnyPic.get(), 0);
	iface->DrawIndexed(info.ActiveCount * 6, 0);
}

void KJohnnyJiantdick::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount);
	JohnnyPic = GetRenderInterface()->LoadTexture2D("johnnyjiantdick");
}

bool KJohnnyJiantdick::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;	
}


void KBulletHole::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindTexture2D(Texture.get(), 0);
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullBack);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.BulletHole.get());
	iface->DrawIndexed(info.ActiveCount * 6, 0);
}

void KBulletHole::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount);
	Texture = GetRenderInterface()->LoadTexture2D("game/bullet_hole");
}

bool KBulletHole::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KBloodTrail::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.BloodTrail.get());
	iface->DrawIndexed(info.ActiveCount * 6 * BLOOD_TRAIL_PARTICLE_COUNT, 0);
}

void KBloodTrail::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * BLOOD_TRAIL_PARTICLE_COUNT);
}

bool KBloodTrail::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KBloodTrail_UnderWater::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.BloodTrail_UnderWater.get());
	iface->DrawIndexed(info.ActiveCount * 6 * BLOOD_TRAIL_PARTICLE_COUNT, 0);
}

void KBloodTrail_UnderWater::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * BLOOD_TRAIL_PARTICLE_COUNT);
}

bool KBloodTrail_UnderWater::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KTorchFlame::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindTexture2D(KSmokeBeam::Tex1.get(), 0);
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.TorchFlame.get());
	iface->DrawIndexed(info.ActiveCount * 6 * TORCH_FLAME_PARTICLE_COUNT, 0);
	//iface->DisableTransparency();
	//iface->BindRenderTarget(iface->SolidTarget.get(), iface->SceneDepthBuffer.get());
}

void KTorchFlame::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * TORCH_FLAME_PARTICLE_COUNT);
	info.bTransparent = true;
}

bool KTorchFlame::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KPowerupBrain::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindShaderProgram(iface->Shaders.BrainPowerup.get());
	iface->BindTexture2D(Texture.get(), 0);
	iface->BindVertexBuffer(Model->GetVertexBuffer(), info.GpuBuffer.get());
	iface->DrawInstanced(Model->GetVertexCount(), info.ActiveCount);
}

void KPowerupBrain::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	Model = new KModelObj;
	Model->LoadFromFile(KString("icosphere"));
	info.GpuBuffer = GetRenderInterface()->CreateInstanceBufferDynamic(typeSize, elementCount);
	Texture = GetRenderInterface()->LoadTexture2D("brainz");
}

void KDynamicLight::RenderBuffer(const KRenderBufferInfo& info)
{

}

void KDynamicLight::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	const u32 maxLights = 1024;	

	KRenderInterface* iface = GetRenderInterface();

	info.GpuBuffer.reset();
	info.GpuBuffer = iface->CreateStructuredBufferDynamic(typeSize, SB_DYNAMIC_LIGHTS, EShaderStage::All, maxLights);

	iface->BindBuffer(info.GpuBuffer.get());
}

bool KDynamicLight::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KAtomProjectile::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.AtomProjectile.get());
	iface->DrawIndexed(info.ActiveCount * 6 * ATOM_SHELL_COUNT * ATOM_SHELL_PARTICLE_COUNT, 0);
}

void KAtomProjectile::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * ATOM_SHELL_COUNT * ATOM_SHELL_PARTICLE_COUNT);
}

bool KAtomProjectile::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KExplosion::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindTexture2D(KSmokeBeam::Tex1.get(), 0);
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.Explosion.get());
	iface->DrawIndexed(info.ActiveCount * 6 * EXPLODE_PARTICLE_COUNT, 0);
}

void KExplosion::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * EXPLODE_PARTICLE_COUNT);
	info.bTransparent = true;
}

bool KExplosion::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KRocketTrail::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.RocketTrail.get());
	iface->DrawIndexed(info.ActiveCount * 6 * ROCKET_TRAIL_PARTICLE_COUNT, 0);
}

void KRocketTrail::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * ROCKET_TRAIL_PARTICLE_COUNT);
	info.bTransparent = true;
}

bool KRocketTrail::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KHealthCrystal::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->GetRasterizer()->SetCullMode(ECullMode::CullFront);
	iface->BindShaderProgram(iface->Shaders.HealthCrystal.get());
	iface->BindVertexBuffer(Model->GetVertexBuffer(), info.GpuBuffer.get());
	iface->BindTexture2D(Tex1.get(), 0);
	iface->BindTexture2D(Tex2.get(), 1);
	iface->DrawInstanced(Model->GetVertexCount(), info.ActiveCount);
}

void KHealthCrystal::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	Model = new KModelObj;
	Model->LoadFromFile(KString("healthcrystal"));
	info.GpuBuffer = GetRenderInterface()->CreateInstanceBufferDynamic(typeSize, elementCount);
	Tex1 = GetRenderInterface()->LoadTexture2D("kf/effects/smoke_1");
	Tex2 = GetRenderInterface()->LoadTexture2D("kf/effects/waterfoam");
	//info.bTransparent = true;
}

void KPortalTravel::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();

	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.PortalTravel.get());
	iface->DrawIndexed(info.ActiveCount * 6 * PORTAL_TRAVEL_COUNT, 0);
}

void KPortalTravel::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * PORTAL_TRAVEL_COUNT);
	info.bTransparent = true;
}

bool KPortalTravel::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KSmokeSheet::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindTexture2D(Texture.get(), 0);
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.SmokeSheet.get());
	iface->DrawIndexed(info.ActiveCount * 6 * 1, 0);
}

void KSmokeSheet::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * PORTAL_TRAVEL_COUNT);
	info.bTransparent = true;
}

void KBlasterParticle::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.BlasterParticle.get());
	iface->DrawIndexed(info.ActiveCount * 6 * BLASTER_PARTICLE_COUNT, 0);
}

void KBlasterParticle::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * BLASTER_PARTICLE_COUNT);
	info.bTransparent = true;
}

bool KBlasterParticle::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

void KBlastExplosion::RenderBuffer(const KRenderBufferInfo& info)
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindBuffer(info.GpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(info.VertexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.BlasterExplosion.get());
	iface->DrawIndexed(info.ActiveCount * 6 * BLAST_EXPLODE_PARTICLE_COUNT, 0);
}

void KBlastExplosion::CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
{
	info.VertexBuffer.reset();
	info.GpuBuffer.reset();
	info.GpuBuffer = GetRenderInterface()->CreateStructuredBufferDynamic(typeSize, 0, EShaderStage::Vertex, elementCount);
	info.VertexBuffer = GetRenderInterface()->CreateParticleIndexBuffer(elementCount * BLAST_EXPLODE_PARTICLE_COUNT);
	info.bTransparent = true;
}

bool KBlastExplosion::ShouldBeUpdated(const KRenderBufferInfo& info)
{
	return true;
}

#endif