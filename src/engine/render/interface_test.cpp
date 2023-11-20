#if !_SERVER

#if 0

/* a place to test various interface functions without worrying about #include upkeep */

#include "interface/render_interface.h"
#include "interface/buffers/vertex_buffer.h"

#include "vertex_types.h"
#include "backend/d3d11/d3d11_shader.h"
#include "engine/utility/kstring.h"
#include "interface/shader_program.h"
#include "../system/terminal/terminal.h"
#include "interface/buffers/gpu_buffer.h"
#include "input_layout.h"
#include "surface2d.h"
#include "interface/texture2d.h"
#include "../system/time.h"
#include "compiler/compiler.h"
#include "compiler/brush/brush_face.h"
#include "interface/render_target.h"
#include "light.h"
#include "light_spot.h"
#include "engine/math/glm.h"
#include "interface/rasterizer.h"
#include "glm/ext/matrix_transform.hpp"
#include "engine/global/types_container.h"
#include "glm/gtx/string_cast.hpp"
#include "../math/helper.h"
#include "compiler/surface_flags.h"
#include "compiler/bsp/bsp_tree.h"
#include "compiler/bsp/bsp_node.h"
#include "../global/types_ptr.h"
#include "backend/d3d11/d3d11_interface.h"
#include "../game_instance.h"
#include "../../game/testmover.h"
#include "../utility/thread_pool.h"
#include "../collision/broadphase/bvh_grid.h"
#include "scene.h"
#include "bufferslots.h"

#define DRAWBONE 0

#define LIGHTING 0


#define USEBSP 0
#define SHOWBSP 0
#define SHOWWIREFRAME 0
#define SHOWPORTALS 0 && USEBSP
#define DRAWNEGZ 0
#define LEAKLINES 1
#define DRAW_ALL 0
#define TEXATLAS 1

struct TestDrawCall
{
	u32 IndexStart = 0;
	u32 IndexCount = 0;
	DVec3 Normal;
	KString TexName;
	TVector<DVec3> Verts;
	class KTexture2D* Texture = nullptr;
	std::unique_ptr<KRenderTarget> LightTexture;
	KTexture2D* lightmap = nullptr;
	glm::mat4 OrthoView;
	f32 w, h;
	TVector<glm::vec2> lightUVs;
	bool bLit = true;
};



KLightSpot* Light;
u32 indsize = 0;
u32 indoffset = 0;
u32 portalindsize = 0;
u32 portalindoffset = 0;
bool firstone = true;
bool becamecreated = false;
u32 lineindexsize = 0;
TVector<TestDrawCall*> TestDrawCalls;
TVector<TestDrawCall*> PortalTestDrawCalls;

TMap<KBrushFace*, TestDrawCall*> PortalDrawMap;

UPtr<KVertexBuffer> Buff1;
UPtr<KVertexBuffer> Buff2;
UPtr<KVertexBuffer> Buff3;
UPtr<KVertexBuffer> Buff4;

void KRenderInterface::__TestInit()
{
	TVector<TestVertex> verts;
	TVector<u32> indices;

	u32 lightindex = 0;

#if USEBSP
	TVector<KBrushFace*> nodefaces;
	TVector<KLeafPortal*> nodeportals;
	KBspNode* node = KMapCompiler::Get().BspHead->GetHead();
	
	std::function<void(KBspNode*)> gettem = [&](KBspNode* node)
	{
		
		if (!node) return;
		if (node->IsLeaf())
		{
			for (const UPtr<KBrushFace>& f : node->GetLeafFaces())
				nodefaces.push_back(f.get());
			for (KLeafPortal* p : node->GetLeafPortals())
#if DRAWNEGZ
				if (p->Plane.Normal.Equals(DVec3(0, 0,-1)))
#endif
				nodeportals.push_back(p);
		}
		else
		{
			gettem(node->GetFront());
			gettem(node->GetBack());
		}
	};

	gettem(node);

	for (KBrushFace* face : nodefaces)
#else

	CurrentScene = std::make_unique<KRenderScene>();
	CurrentScene->SubmitFaces(KMapCompiler::Get().WorldFaces);

	for (UPtr<KBrushFace>& face : KMapCompiler::Get().WorldFaces)
#endif
	{
		break;/////////////////////////////
			

		if (!face->bFilled) continue;

		TestDrawCall* call = new TestDrawCall();
		if (face->Lightmap.size() > 0)
		{
			u32 w = face->Lightmap.size();
			u32 h = face->Lightmap[0].size();

			UPtr<KSurface2D> surface = std::make_unique<KSurface2D>();
			surface->ClearAndResizeData(w, h);
			
			for (u32 x = 0; x < w; x ++)
			{
				for (u32 y = 0; y < h; y++)
				{
					DVec3 c = face->Lightmap[x][y].Color;
					//c *= 255; // TODO maybe use over 255 as alpha and convert to bloom?
					//c.Clamp(DVec3(0, 0, 0), DVec3(255, 255, 255));
					FColor32 c32(c.x, c.y, c.z, 1);
					surface->SetPixel(x, y, c32.To8());
				}
			}

			face->TestLightmap = CreateTexture2DFromSurface("light_" + KString(lightindex), std::move(surface));
			call->lightmap = face->TestLightmap;
			lightindex++;
		}

		TestDrawCalls.push_back(call);
		call->IndexStart = indsize;
		call->IndexCount = face->Indices.size();
		call->Texture = face->Texture;
		call->Normal = face->Plane.Normal;
		call->TexName = face->TexName;
		call->bLit = true;//face->Surface & u32(ESurfaceFlags::NO_DRAW);
		//if (!call.bLit) continue;
		for (KBrushVertex& v : face->Vertices)
		{
			call->Verts.push_back( { v.Point.x, v.Point.y, v.Point.z } );
		}

#if DRAWBONE
		if (call.TexName.Contains("bone"))
		GetViewOverVertices(call.Verts, glm::mat4(1.0f), call->OrthoView, call->w, call->h, call->lightUVs);
		else
		call.lightUVs.resize(call.Verts.size());
#else
		GetViewOverVertices(call->Verts, glm::mat4(1.0f), call->OrthoView, call->w, call->h, call->lightUVs);
#endif

		int i = 0;
		for (KBrushVertex& v : face->Vertices)
		{
			verts.push_back({ f32(v.Point.y), f32(v.Point.z), f32(v.Point.x), f32(v.TexU), f32(v.TexV),
				f32(v.Normal.y), f32(v.Normal.z), f32(v.Normal.x),
				//call->lightUVs[i].x, call->lightUVs[i].y });
				v.LocalLightU, v.LocalLightV,
				f32(v.ClampU.x), f32(v.ClampU.y), f32(v.ClampV.x), f32(v.ClampV.y) });
			i++;
		}

		for (u32 i : face->Indices)
			indices.push_back(i + indoffset);

		indoffset += face->Vertices.size();
		indsize += face->Indices.size();
		face->__TestDrawCall = call;
	}

	indsize = indices.size();
	//Buff1 = CreateVertexBufferStatic(verts.data(), sizeof(TestVertex), verts.size(), indices);
	
	TVector<TestVertex> verts1 =
	{
		{ -1.0f,  1.0f,  0.0f,		0.f, 0.f,		0.f, 0.f, 0.f,		0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
		{  1.0f,  1.0f,  0.0f,		1.f, 0.f,		0.f, 0.f, 0.f,		0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
		{  1.0f, -1.0f,  0.0f,		1.f, 1.f,		0.f, 0.f, 0.f,		0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
		{ -1.0f, -1.0f,  0.0f,		0.f, 1.f,		0.f, 0.f, 0.f,		0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
	};
	TVector<u32> indices1 = { 0, 1, 2,	3, 0, 2 };
	Buff2 = CreateVertexBufferStatic(verts1.data(), sizeof(TestVertex), verts1.size(), indices1);

#if SHOWPORTALS
	TVector<TestVertex> verts2;
	TVector<u32> indices2;
	for (auto& portal : KMapCompiler::Get().Portals/*nodeportals*/)
	{
		TestDrawCall* call = new TestDrawCall();
		PortalTestDrawCalls.push_back(call);
		call->IndexStart = portalindsize;
		call->IndexCount = portal->Indices.size();

		for (KBrushVertex& v : portal->Vertices)
		{
			verts2.push_back({ f32(v.Point.y), f32(v.Point.z), f32(v.Point.x), 0, 0, 0, 0, 0, 0, 0 });
		}

		for (u32 i : portal->Indices)
		{
			indices2.push_back(i + portalindoffset);
		}
		portalindoffset += portal->Vertices.size();
		portalindsize += portal->Indices.size();

		PortalDrawMap[portal.get()] = call;
	}
	Buff3 = CreateVertexBufferStatic(verts2.data(), sizeof(TestVertex), verts2.size(), indices2);
#endif

#if LEAKLINES
	TVector<TestVertex> verts3;
	TVector<u32> indices3;
	u32 theindex = 0;
	for (const DVec3& v : KMapCompiler::Get().LeakPath)
	{
		verts3.push_back({ f32(v.y), f32(v.z), f32(v.x), 0, 0, 0, 0, 0, 0, 0 });

		indices3.push_back(theindex);

		theindex++;
	}
	lineindexsize = indices3.size();
	if (lineindexsize > 0)
		Buff4 = CreateVertexBufferStatic(verts3.data(), sizeof(TestVertex), verts3.size(), indices3);
#endif

	// create transform buffer
	//ModelViewProjectionBuffer = CreateConstantBufferStatic(sizeof(glm::mat4), 0, EShaderStage::Vertex);
	//ModelTransformBuffer = CreateConstantBufferStatic(sizeof(glm::mat4), 1, EShaderStage::Vertex);
	//LightMatrixBuffer = CreateConstantBufferStatic(sizeof(glm::mat4), 2, EShaderStage::Vertex);
	PositionBuffer = CreateConstantBufferStatic(sizeof(glm::vec4) * 3, 3, EShaderStage::Pixel);
	//TimeConstantBuffer = CreateConstantBufferStatic(sizeof(f32) * 4, CB_TIME, EShaderStage::None);

	//FRenderTargetCreationFlags f;
	//PostTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);

	// bind them
	BindShaderProgram(Program1.get());
	//BindVertexBuffer(Buff1.get());
	BindConstantBuffer(ModelViewProjectionBuffer.get());
	BindConstantBuffer(ModelTransformBuffer.get());
	BindConstantBuffer(LightMatrixBuffer.get());
	BindConstantBuffer(PositionBuffer.get());
	BindConstantBuffer(FrameDataBuffer.get());
	
	becamecreated = true;
	Light = new KLightSpot();
}

glm::vec4 lastcam;

void KRenderInterface::__Test()
{	
	using namespace glm;
	if (!becamecreated) return;
	//if (indsize == 0) return;

	DVec3 camera(Camera.GetPosition().z, Camera.GetPosition().x, Camera.GetPosition().y);
	DVec3 lastcamera(lastcam.z, lastcam.x, lastcam.y);
	DVec3 dir(Camera.GetDirection().z, Camera.GetDirection().x, Camera.GetDirection().y);
	DVec3 target = camera + (dir * 4000);

	DHitResult testhit;

#if 0
	FTimePoint starter = KTime::Now();
	bool jhohnny = KMapCompiler::Get().Grid->TraceLine(DLineSegment(camera, target), testhit);
	f64 ender = KTime::Since(starter);
	SYSLOG(std::fixed << std::setprecision(10) << ender);
#endif

	/*if (camera != lastcamera)
	{
		KMapCompiler::Get().BspHead->TraceSphere(DLineSegment(lastcamera, camera), 16, &testhit);
	}

	if (testhit.bHit)
	{
		

		//DVec3 adjust = lastcamera;//testhit.Point + (testhit.Normal * 17);
		DVec3 adjust = testhit.Point;
		DVec3 remaining = 1 - testhit.Time;


		FVec3 fadjust(adjust.x, adjust.y, adjust.z);
		glm::vec3 g3 = fadjust.ToGLM();
		Camera.CameraData.Position = glm::vec4(g3.x, g3.y, g3.z, 1);
		//Camera.CameraData.Position = glm::vec4(0, 0, 0, 1);

	}*/

	testhit.bHit = false;
	//KMapCompiler::Get().BspHead->TraceSphere(DLineSegment(camera, target), 16, &testhit);
	
		/*FTimePoint startt = KTime::Now();
		KMapCompiler::Get().BspHead->TraceSphere(DLineSegment(camera, target), 16, &testhit);
		//KMapCompiler::Get().BspHead->TraceLine(DLineSegment(camera, target), &testhit);
		f64 endt = KTime::Since(startt);
		SYSLOG(std::setprecision(12) << std::fixed << endt << std::scientific);*/

	lastcam = Camera.GetPosition();

	struct
	{
		f32 Time;
		f32 Pitch;
		f32 extra[2];
	} thedata;
	thedata.Time = KTime::SinceInit();
	thedata.Pitch = Camera.LastPitch * (180.0 / PI<f64>());
	UpdateConstantBuffer(FrameDataBuffer.get(), &thedata);

	// create model matrix
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0, 0, 0));

	FRenderTargetCreationFlags f;

#if !LIGHTING
	firstone = false;
#endif

	/*if (firstone)
	{
		struct positionbuffer
		{
			glm::vec4 lightpos, viewpos, lightfacing;
		};

		positionbuffer pbuff;
		pbuff.lightpos = glm::vec4(0, 0, 0, 1);
		pbuff.viewpos = Camera.GetPosition();
		pbuff.lightfacing = glm::vec4(Light->LightDirection, 1);
		UpdateConstantBuffer(PositionBuffer.get(), &pbuff);

		UpdateConstantBuffer(ModelTransformBuffer.get(), &model);
		Rasterizer->SetCullMode(ECullMode::CullBack);
		Light->DepthBuffer->Clear();
		Light->__BuildTestMatrix();
	
		auto tar = CreateRenderTarget(2048, 2048, f);
		SetViewport(2048, 2048);
		tar->Clear(1, 1, 1, 1);
		BindRenderTarget(tar.get(), Light->DepthBuffer.get()); // dummy target, we want to render to depth
		BindShaderProgram(Program3.get());
		//BindVertexBuffer(Buff1.get());
		glm::mat4 mvplight = glm::transpose(Light->LightMatrix * model);
		UpdateConstantBuffer(TransformConstantBuffer.get(), &mvplight);
	
		for (TestDrawCall* call : TestDrawCalls)
		{
			BindTexture2D(call->Texture, 0);
			DrawIndexed(call->IndexCount, call->IndexStart);
		}
	}*/

	glm::mat4 mvp;

#if !DRAWBONE
	if (!firstone)
	{	
		//GetD3D11ImmediateContext()->ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

		struct positionbuffer
		{
			glm::vec4 lightpos, viewpos, lightfacing;
		};

		positionbuffer pbuff;
		pbuff.lightpos = glm::vec4(0, 0, 0, 1);
		pbuff.viewpos = Camera.GetPosition();
		pbuff.lightfacing = glm::vec4(Light->LightDirection, 1);
		UpdateConstantBuffer(PositionBuffer.get(), &pbuff);
		UpdateConstantBuffer(ModelTransformBuffer.get(), &model);

		mvp = glm::transpose(Camera.GetViewProjectionMatrix() * model);
		// feed mvp matrix to shader
		UpdateConstantBuffer(ModelViewProjectionBuffer.get(), &mvp);
		UpdateConstantBuffer(LightMatrixBuffer.get(), &Light->LightMatrix);

		//BindBackBuffer(true);
		BindRenderTarget(PostTarget.get(), SceneDepthBuffer.get());
		SetViewport(GetViewportX(), GetViewportY());
		//Light->DepthBuffer->BindTexture2D(1);
		
		TVector<TestDrawCall*> calls;
		
#if 1
		//BindVertexBuffer(Buff1.get());
		//QuadTarget->Clear(.9, .9, 1, 1);
		PostTarget->Clear(.1, .1, .1, 1);

#if SHOWWIREFRAME
		Rasterizer->SetFillMode(EFillMode::Wireframe);
		Rasterizer->SetCullMode(ECullMode::CullBack);
		BindTexture2D(0, 0);
		BindShaderProgram(ShaderWorldWireframe.get());
		DrawIndexed(indsize, 0);
#endif

		//Rasterizer->SetCullMode(ECullMode::CullNone);
		//Rasterizer->SetFillMode(EFillMode::Wireframe);
		//Rasterizer->SetFillMode(EFillMode::Solid);
		//Rasterizer->SetCullMode(ECullMode::CullBack);
		
		BindWorldShader();

#if USEBSP
		KBspNode* node = KMapCompiler::Get().BspHead->GetHead();
		TVector<KBrushFace*> nodefaces;

		FVec3 campos;
		glm::vec<3, f64> glmpos(Camera.GetPosition().x, Camera.GetPosition().y, Camera.GetPosition().z);

		while (!node->IsLeaf())
		{
			if (node->GetPlane().ClassifyPoint(DVec3::FromGLM(glmpos)) == EPointSide::Front)
			{
				node = node->GetFront();
			}
			else
			{
				node = node->GetBack();
			}
		}

		for (const UPtr<KBrushFace>& face : node->GetLeafFaces())
		{
			calls.push_back((TestDrawCall*)face->__TestDrawCall);
		}
#endif
	
#if 0 && (DRAW_ALL || TEXATLAS)
#if TEXATLAS
		BindShaderProgram(Program4.get());
		BindTexture2D(KMapCompiler::Get().TextureAtlas, 0);
#else
		BindTexture2D(0, 0);
#endif
		BindShaderProgram(Program4.get());
		DrawIndexed(indsize, 0);
#else
#if USEBSP && SHOWBSP
		for (TestDrawCall* call : calls)
#else

		BindWorldShader();
		BindTexture2D(KMapCompiler::Get().TextureAtlas, 0);
		DrawStaticWorld();

		for (TestDrawCall* call : TestDrawCalls)
#endif
		{
			break;///////////////////////////////////////

			if (call->TexName.Contains("stain"))
				BindShaderProgram(Program5.get());
			else 
				BindWorldShader();
				//BindShaderProgram(Shaders.ActiveWorld);

			//BindTexture2D(call->Texture, 0);
			BindTexture2D(KMapCompiler::Get().TextureAtlas, 0);
#if LIGHTING
			if (call->bLit) 
				call->LightTexture->BindTexture2D(2);
			else
#endif
				
			if (call->lightmap)
				BindTexture2D(call->lightmap, 2);
			else
				BindTexture2D(0, 2);

#if !DRAWNEGZ
			DrawIndexed(call->IndexCount, call->IndexStart);
#endif
		}
#endif

#if SHOWPORTALS
		Rasterizer->SetFillMode(EFillMode::Wireframe);
		Rasterizer->SetCullMode(ECullMode::CullNone);
		BindShaderProgram(Program7.get());
		BindVertexBuffer(Buff3.get());
#if SHOWBSP
		for (KBrushFace* p : node->LeafPortals)
		{
			TestDrawCall* pc = PortalDrawMap[p];
			DrawIndexed(pc->IndexCount, pc->IndexStart);
		}
#else
		DrawIndexed(portalindsize, 0);
#endif
#endif

#if LEAKLINES
		if (lineindexsize > 0)
		{
			GetD3D11ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			Rasterizer->SetFillMode(EFillMode::Solid);
			Rasterizer->SetCullMode(ECullMode::CullNone);
			BindShaderProgram(Program7.get());
			BindVertexBuffer(Buff4.get());
			DrawIndexed(lineindexsize, 0);

			if (testhit.bHit)
			{
				UPtr<KVertexBuffer> liner;
				FVec3 hitpoint = testhit.Point.ToType<f32>();
				FVec3 hitnorm = (testhit.Point + (testhit.Normal * 16)).ToType<f32>();
				TVector<TestVertex> lineverts = 
				{
					{ hitpoint.y, hitpoint.z, hitpoint.x, .0f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, },
					{ hitnorm.y, hitnorm.z, hitnorm.x, .0f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, },
				};
				TVector<u32> inds = { 0, 1 };
				auto newbuff = CreateVertexBufferStatic(lineverts.data(), sizeof(TestVertex), lineverts.size(), inds);
				BindVertexBuffer(newbuff.get());
				DrawIndexed(2, 0);
			}

			GetD3D11ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
#endif

		/*if (KTestMover* mover = KGameInstance::Get().TestMover)
		{
			//if (mover->HitNormal != DVec3(0, 0, 0))
			{
				GetD3D11ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				Rasterizer->SetFillMode(EFillMode::Solid);
				Rasterizer->SetCullMode(ECullMode::CullNone);
				BindShaderProgram(Program7.get());

				UPtr<KVertexBuffer> liner;
				FVec3 hitpoint = mover->HitPoint.ToType<f32>();
				FVec3 hitnorm = (mover->HitPoint + (mover->HitNormal * 4)).ToType<f32>();
				TVector<TestVertex> lineverts =
				{
					{ hitpoint.y, hitpoint.z, hitpoint.x, .0f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, },
					{ hitnorm.y, hitnorm.z, hitnorm.x, .0f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, },
				};
				TVector<u32> inds = { 0, 1 };
				auto newbuff = CreateVertexBufferStatic(lineverts.data(), sizeof(TestVertex), lineverts.size(), inds);
				BindVertexBuffer(newbuff.get());
				DrawIndexed(2, 0);

				GetD3D11ImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				//SYSLOG(hitpoint.ToString());
			}
		}*/


		// render image to quad
		/*Rasterizer->SetFillMode(EFillMode::Solid);
		Rasterizer->SetCullMode(ECullMode::CullNone);
		BindShaderProgram(Program2.get());
		BindBackBuffer(false);
		SetViewport(GetViewportX(), GetViewportY());
		//BindVertexBuffer(Buff2.get());
		BindVertexBuffer(nullptr);
		PostTarget->BindTexture2D(0);
		//DrawIndexed(6, 0);
		Draw(3, 0);
		BindTexture2D(0, 0);*/

#else
		BindTexture2D(0, 0);
		BindShaderProgram(ShaderWorldLit.get());
		DrawIndexed(indsize, 0);
#endif
	}
	else
#endif
	{

		for (TestDrawCall* call : TestDrawCalls)
		{
#if DRAWBONE
			if (!call->TexName.Contains("bone")) continue;
#endif
			if (!call->bLit) continue;
			f32 scale = 1.f / 4.f;

			f32 w, h;
			w = call->w * scale;
			h = call->h * scale;
			w = std::clamp(w, 1.f, 2048.f);
			h = std::clamp(h, 1.f, 2048.f);

			UpdateConstantBuffer(ModelViewProjectionBuffer.get(), &(call->OrthoView));
			UpdateConstantBuffer(LightMatrixBuffer.get(), &Light->LightMatrix);

			BindVertexBuffer(Buff1.get());
			BindShaderProgram(Program1.get());
			if (!call->LightTexture) call->LightTexture = CreateRenderTarget(w, h, f);
			call->LightTexture->Clear(0, 0, 0, 1);
			BindRenderTarget(call->LightTexture.get(), nullptr);
			Light->DepthBuffer->BindTexture2D(1);
			SetViewport(w, h);

			BindTexture2D(call->Texture, 0);
			DrawIndexed(call->IndexCount, call->IndexStart);

#if DRAWBONE
			BindShaderProgram(Program2.get());
			BindBackBuffer(false);
			SetViewport(w, h);
			BindVertexBuffer(Buff2.get());
			call->LightTexture->BindTexture2D(0);
			DrawIndexed(6, 0);
			BindTexture2D(0, 0);
#endif			
		}
	}
	
	BindTexture2D(nullptr, 1);

	firstone = false;
}

#endif

#endif