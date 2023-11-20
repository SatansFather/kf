#if !_SERVER

#include "scene.h"
#include "interface/render_interface.h"
#include "interface/buffers/vertex_buffer.h"
#include "engine/render/precipitation.h"
#include "compiler/brush/brush_vertex.h"
#include "compiler/brush/brush_face.h"
#include "bufferslots.h"
#include "interface/texture2d.h"
#include "../../compiler/compiler.h"
#include "interface/rasterizer.h"
#include "material.h"
#include "interface/render_target.h"

// DELETE
#include "font.h"

struct KPendingMatBuffer
{
	KMaterial* Material = nullptr;
	u32 TypeSize = 0;
	TVector<u8> Buffer;

	UPtr<KGpuBuffer> CreateBuffer()
	{
		auto buff = GetRenderInterface()->CreateStructuredBufferStatic(TypeSize, MAT_BUFFER_SLOT, EShaderStage::Pipeline, Buffer.size() / TypeSize);
		buff->Update(Buffer.data(), Buffer.size() / TypeSize);
		return buff;
	}
};

TVector<KPendingMatBuffer> PendingMaterialBufferData;

KRenderScene::KRenderScene()
{
	PendingMaterialBufferData.clear();
}

KRenderScene::~KRenderScene() {}

template <typename T>
void CreateBufferForEntry(TVector<TMap<KString, KString>>& data, KMaterial* mat)
{
	TVector<u8> buffData;
	for (auto m : data)
	{
		T d;
		d.InitFromMap(m);
		buffData.insert(buffData.end(), sizeof(T), 0);
		memcpy(buffData.data() + buffData.size() - sizeof(T), &d, sizeof(T));
	}

	KPendingMatBuffer b;
	b.Material = mat;
	b.TypeSize = sizeof(T);
	b.Buffer = std::move(buffData);
	PendingMaterialBufferData.push_back(std::move(b));
}

#if _COMPILER
void KRenderScene::SubmitFaces(TVector<UPtr<class KBrushFace>>& faces)
{
	OpaqueDrawCalls.clear();
	TransparentDrawCalls.clear();
	WaterDrawCalls.clear();
	StaticVertexBuffer.reset();
	WorldVertices.clear();
	WorldIndices.clear();
	DrawCallsIndexOffset = 0;

	u32 indoffset = 0;
	u32 indsize = 0;
	u32 vertsize = 0;

	// sort faces by material
	TMap<KString, TVector<KBrushFace*>> sortedFaces;
	for (UPtr<KBrushFace>& face : faces)
		sortedFaces[face->Material].push_back(face.get());

	//for (UPtr<KBrushFace>& face : faces)
	for (const auto kv : sortedFaces)
	{
		StaticBufferDrawCall call;
		call.Material = GetRenderInterface()->Materials.GetByName(kv.first);

		for (KBrushFace* face : kv.second)
		{
			u32 bufferIndex = 0;
			if (!face->Material.IsEmpty())
			{
				call.PendingMaterialProperties.push_back(face->GetMaterialPropertyMap());
				bufferIndex = call.PendingMaterialProperties.size() - 1;
			}

			for (KBrushVertex& v : face->Vertices)
			{
				SceneBounds.Update(v.Point.ToType<f32>());

				KWorldVertex vert;

				vert.PosX = v.Point.y;
				vert.PosY = v.Point.z;
				vert.PosZ = v.Point.x;

				vert.NormX = v.Normal.y;
				vert.NormY = v.Normal.z;
				vert.NormZ = v.Normal.x;

				vert.TexU = v.TexU;
				vert.TexV = v.TexV;

				vert.LightU = v.LightU;
				vert.LightV = v.LightV;

				vert.MinTexU = v.ClampU.x;
				vert.MaxTexU = v.ClampU.y;

				vert.MinTexV = v.ClampV.x;
				vert.MaxTexV = v.ClampV.y;

				vert.Red = v.Red;
				vert.Green = v.Green;
				vert.Blue = v.Blue;
				
				vert.BufferIndex = bufferIndex;

				WorldVertices.push_back(vert);
			}

			for (u32 i : face->Indices)
				WorldIndices.push_back(i + indoffset);

			indoffset += face->GetVertices().size();
			vertsize += face->GetVertices().size();
			indsize += face->GetIndices().size();
			call.IndexCount += face->GetIndices().size();
		}

		if (call.PendingMaterialProperties.size() > 0)
		{
			CreateMaterialBuffer(call.Material, call.PendingMaterialProperties);
			
			// compiler doesnt need to wait to do this like game does since render isnt in a separate thread
			CreateMaterialBufferForCall(call);
		}

		SubmitStaticDrawCall(std::move(call));
		//StaticDrawCalls.push_back(call);
	}
	
	StaticVertexCount = WorldVertices.size();
	StaticIndexCount = WorldIndices.size();
	StaticVertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(
		WorldVertices.data(), sizeof(KWorldVertex), WorldVertices.size(), WorldIndices);
}
#endif

void KRenderScene::SubmitVertices(const TVector<KWorldVertex>& verts, const TVector<u32>& inds)
{
	WorldVertices = verts;
	WorldIndices = inds;
	StaticVertexCount = verts.size();
	StaticIndexCount = inds.size();
	for (const KWorldVertex& v : verts)
		SceneBounds.Update(FVec3(v.PosZ, v.PosX, v.PosY));
	StaticVertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(
		WorldVertices.data(), sizeof(KWorldVertex), WorldVertices.size(), WorldIndices);
}

void KRenderScene::SubmitStaticDrawCall(StaticBufferDrawCall call)
{
	call.IndexOffset = DrawCallsIndexOffset;

	u32 indexCount = call.IndexCount;

	if (call.Material->MaterialName == "water")
		WaterDrawCalls.push_back(std::move(call));
	else if (call.Material->bTransparent)
		TransparentDrawCalls.push_back(std::move(call));
	else
		OpaqueDrawCalls.push_back(std::move(call));

	DrawCallsIndexOffset += indexCount;
}

void KRenderScene::DrawOpaque(bool sky /*= false*/, KRenderTarget* target)
{
	KRenderInterface* iface = GetRenderInterface();

	if (target)
	{
		iface->BindRenderTarget(target, iface->SceneDepthBuffer.get());
	}
	else
	{
#if 1
		iface->BindRenderTarget(iface->SolidTarget.get(), iface->SceneDepthBuffer.get());
		//KRenderTarget* targets[2] = { iface->SolidTarget.get(), iface->BrightPixelsTarget.get() };
		//iface->BindRenderTargets(targets, 2, iface->SceneDepthBuffer.get());
#else
		iface->BindGBuffer();
#endif
	}

	iface->SetMvpFromModel(glm::mat4(1.f));
	if (sky)
	{
		iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
		iface->GetRasterizer()->SetCullMode(ECullMode::CullBack);
		iface->BindShaderProgram(iface->Shaders.WorldUnlit.get());

		iface->BindTexture2D(TextureAtlas.get(), 0);
		iface->BindTexture2D(LightmapTexture.get(), 2);
		iface->BindVertexBuffer(GetVertexBuffer());
		iface->DrawIndexed(GetIndexCount(), 0);
		return;
	}
	
	iface->DisableTransparency();
	u32 indexOffset = 0;
	iface->BindVertexBuffer(GetVertexBuffer());
	for (StaticBufferDrawCall& call : OpaqueDrawCalls)
	{
		CreateMaterialBufferForCall(call);

		if (call.IndexCount == 0) continue;

		call.Material->Bind();
		if (call.Buffer)
			iface->BindBuffer(call.Buffer.get());

		iface->BindTexture2D(TextureAtlas.get(), 0);
		iface->BindTexture2D(LightmapTexture.get(), 2);
		iface->DrawIndexed(call.IndexCount, call.IndexOffset);
		indexOffset += call.IndexCount;
	}


	//iface->BindWorldShader();
	//iface->BindTexture2D(KMapCompiler::Get().TextureAtlas, 0);
	//iface->BindTexture2D(KMapCompiler::Get().LightmapTexture.get(), 2);
	//iface->BindVertexBuffer(GetVertexBuffer());
	//iface->DrawIndexed(GetIndexCount(), 0);
}

void KRenderScene::DrawTransparent(bool sky /*= false*/)
{
	
}

void KRenderScene::DrawWater(bool sky /*= false*/)
{
	if (WaterDrawCalls.size() == 0) return;

	KRenderInterface* iface = GetRenderInterface();
	u32 indexOffset = 0;
	iface->SetMvpFromModel(glm::mat4(1.f));
	iface->BindVertexBuffer(GetVertexBuffer());
	iface->SetDepthBufferReadOnly(true);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindTexture2D(TextureAtlas.get(), 0);
	iface->BindTexture2D(LightmapTexture.get(), 1);

	iface->BindShaderProgram(iface->Shaders.Water.get());

#if !_COMPILER
	KRenderTarget* targets[2] =
	{
		iface->WaterTarget.get(),
		iface->WaterDepthTarget.get()
	};
	iface->BindRenderTargets(targets, 2, iface->SceneDepthBuffer.get());
#endif
	iface->EnableWaterBlend();

	for (StaticBufferDrawCall& call : WaterDrawCalls)
	{
		CreateMaterialBufferForCall(call);
		if (call.IndexCount == 0) continue;

		if (call.Buffer)
			iface->BindBuffer(call.Buffer.get());

		//call.Material->Bind();
		iface->DrawIndexed(call.IndexCount, call.IndexOffset);
		indexOffset += call.IndexCount;
	}

#if !_COMPILER
	targets[0] = iface->SolidTarget.get();
	targets[1] = nullptr;
	iface->BindRenderTargets(targets, 2, iface->SceneDepthBuffer.get());
#endif
	iface->ClearBlendState();
}

void KRenderScene::AddPrecipitation(const class KPrecipitationVolume& p)
{
	// create a precipitation buffer from volume
	UPtr<KPrecipitationBuffer> buffer = std::make_unique<KPrecipitationBuffer>();
	buffer->Init(p);
	Precipitation[p.GetID()] = std::move(buffer);
}

void KRenderScene::DrawPrecipitation()
{
	for (const auto& kv : Precipitation)
		kv.second->Draw();
}

void KRenderScene::LoadLeakLines()
{
#if 0
	TVector<KLeakVertex> verts;
	TVector<u32> indices;
	u32 index = 0;
	for (const DVec3& v : KMapCompiler::Get().LeakPath)
	{
		verts.push_back({ f32(v.y), f32(v.z), f32(v.x) });
		indices.push_back(index);
		index++;
	}
	LeakLineIndexCount = indices.size();
	if (LeakLineIndexCount > 0)
		LeakLineVertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(verts.data(), sizeof(KLeakVertex), verts.size(), indices);
#endif
}

void KRenderScene::DrawLeakLines()
{
	KRenderInterface* iface = GetRenderInterface();
	if (iface && LeakLineIndexCount > 0)
	{
		iface->SetTopology_LineStrip();
		iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
		iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
		iface->BindShaderProgram(iface->Shaders.LeakLine.get());
		iface->BindVertexBuffer(LeakLineVertexBuffer.get());
		iface->DrawIndexed(LeakLineIndexCount, 0);
		iface->SetTopology_TriangleList();
	}
}

void KRenderScene::SetupLeakLineVertexBuffer()
{
	TVector<KLeakVertex> verts;
	TVector<u32> indices;
	u32 index = 0;
	for (const DVec3& v : LeakPath)
	{
		verts.push_back({ f32(v.y), f32(v.z), f32(v.x) });
		indices.push_back(index);
		index++;
	}
	LeakLineIndexCount = indices.size();
	if (LeakLineIndexCount > 0)
		LeakLineVertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(verts.data(), sizeof(KLeakVertex), verts.size(), indices);
}

void KRenderScene::CreateLightmap(const TVector<u32>& colors, u32 w, u32 h)
{
	
}

void KRenderScene::CreateMaterialBuffer(KMaterial* mat, TVector<TMap<KString, KString>>& m)
{
	// TODO water InitFromMap is not happening

	if (mat->MaterialName == "portal")
	    CreateBufferForEntry<KPortalData>(m, mat);
	else if (mat->MaterialName == "water")
	    CreateBufferForEntry<KWaterData>(m, mat);
}

void KRenderScene::CreateMaterialBufferForCall(StaticBufferDrawCall& call)
{
	for (i32 i = PendingMaterialBufferData.size() - 1; i >= 0; i--)
	{
		if (call.Material == PendingMaterialBufferData[i].Material)
		{
			if (call.Material)
				call.Buffer = PendingMaterialBufferData[i].CreateBuffer();
#if !_COMPILER
			VectorRemoveAt(PendingMaterialBufferData, i);
#endif
			return;
		}
	}
}

#endif
