#if !_SERVER


#include "render_interface.h"
#include "engine/game_instance.h"
#include "engine/utility/k_assert.h"
#include "rasterizer.h"
#include "window.h"
#include "depth_buffer.h"
#include "shader_program.h"
#include "GLFW/glfw3.h"
#include "buffers/gpu_buffer.h"
#include "texture2d.h"
#include "engine/render/communication.h"
#include "render_target.h"
#include "../hud/hud_engine_console.h"
#include "../scene.h"
#include "../bufferslots.h"
#include "buffers/vertex_buffer.h"
#include "blend_state.h"
#include "engine/render/precipitation.h"
#include "bitmap.h"

// DELETE
#include "engine/system/terminal/terminal.h"
#include "../../../compiler/compiler.h"
#include "../particle.h"
#include "compiler/surface_flags.h"
#include "../material.h"

#if !_COMPILER
#include "../../console/engine_console.h"
#include "engine/render/hud/hud_frame_stats.h"
#include "engine/render/hud/hud_net_stats.h"
#include "engine/render/hud/hud_game_frame_stats.h"
#include "../hud/hud_powerup_timers.h"
#include "../hud/hud_load_screen.h"
#include "../hud/hud_chat_console.h"
#include "../hud/hud_menu.h"
#include "engine/menus/basic_menu.h"
#include "../hud/hud_character_info.h"
#include "../hud/hud_game_timer.h"
#include "../hud/hud_damage_numbers.h"
#include "../hud/hud_target_name.h"
#include "../hud/hud_scoreboard.h"
#include "../hud/hud_frag_message.h"
#include "../hud/hud_kill_feed.h"
#include "../font.h"
#include "../hud/hud_on_screen_message.h"
#include "../hud/hud_pickup_message.h"
#endif

#define GI_RESOLUTION 16
#define SAVE_GI_SCREENSHOT 0

KRenderInterface::KRenderInterface(bool new_thread /*= true*/) 
	: bRenderThread(new_thread)
{
	
}

// defined here so we can forward declare unique pointer types
KRenderInterface::~KRenderInterface() {}

KRenderInterface* GetRenderInterface()
{
#if _COMPILER
	return KMapCompiler::Get().RenderInterface.get();
#else
	return KGameInstance::Get().GetRenderInterface();
#endif
}

f32 GetMaxRenderScale()
{
	KRenderInterface* i = GetRenderInterface();
	f32 maxScale = i->GetDrawConfig()->GetRenderScale();

	if (GetDrawConfig()->ScaleRenderByArea())
	{
		const f32 pixels = GetViewportPixelCount();
		const f32 targetPixels = GetViewportPixelCount() * maxScale;
		const f32 areaFactor = targetPixels / pixels;
		const f32 linearFactor = sqrt(areaFactor);
		maxScale = linearFactor;
	}

	if (maxScale > 1)
	{
		const f32 maxDimension = 16384; // d3d11 texture size limit
		const f32 h = i->GetGameWindow()->GetHeight();
		const f32 w = i->GetGameWindow()->GetWidth();
		const f32 biggest = maxScale * KMax(w, h);
		if (biggest > maxDimension) maxScale *= maxDimension / biggest;
	}
	return maxScale;
}

f32 GetRenderResX()
{
#if _COMPILER
	return GI_RESOLUTION;
#endif

	return GetViewportX() * GetMaxRenderScale();
}

f32 GetRenderResY()
{
#if _COMPILER
	return GI_RESOLUTION;
#endif

	return GetViewportY() * GetMaxRenderScale();
}

f32 GetViewportX()
{
#if _COMPILER
	return 1;
#endif
	return GetRenderInterface()->GetGameWindow()->GetWidth();
}

f32 GetViewportY()
{
#if _COMPILER
	return 1;
#endif
	return GetRenderInterface()->GetGameWindow()->GetHeight();
}

f32 GetScaledX()
{
#if _COMPILER
	return 1;
#endif
	const f32 baseX = 1600.f;
	return GetViewportX() / baseX;
}

f32 GetScaledY()
{
#if _COMPILER
	return 1;
#endif
	const f32 baseY = 900.f;
	return GetViewportY() / baseY;
}

f32 GetRenderScale()
{
#if _COMPILER
	return 1;
#endif
	return GetRenderInterface()->GetDrawConfig()->GetRenderScale();
}

f32 GetViewportPixelCount()
{
	return GetViewportX() * GetViewportY();
}

void KRenderInterface::InitializeInterface()
{
	// create window in this thread 
	
#if _COMPILER
	//CreateAndSetGameWindow(1, 1, 1);
	CreateAndSetGameWindow(GI_RESOLUTION * 2, GI_RESOLUTION * 2, (u8)EWindowState::Windowed);
#else
	u32 x = GetDrawConfig()->GetResX();
	u32 y = GetDrawConfig()->GetResY();

	LOG("Creating window at " + KString(x) + "x" + KString(y));
	CreateAndSetGameWindow(x, y, (u8)GetDrawConfig()->GetWindowState());
#endif

#if !_COMPILER
		RenderThread = std::thread(&KRenderInterface::StartRenderThread, this);
#else
		FinishInit();
#endif
}

void KRenderInterface::StartRenderThread()
{
	FinishInit();

	// start rendering
	DrawLoop();
}

void KRenderInterface::FinishInit()
{
	InitializeBackend();

	Rasterizer = CreateRasterizerState();
	Rasterizer->SetCullMode(ECullMode::CullBack);
	Rasterizer->SetFillMode(EFillMode::Solid);

	SceneDepthBuffer = CreateDepthBuffer();
	SceneDepthBuffer->SetReadOnly(false);
	SceneDepthBuffer->SetEnabled(true);

	CreateShaders();
	BindBackBuffer();
	SetViewport(GetViewportX(), GetViewportX());
	InitConstantBuffers();
	LoadShaders();
	CreateMaterials();
	LoadFonts();
	CreateRenderTargets();
	CreateBlendStates();
#if !_COMPILER
	UpdateRenderScaleSampler(GetDrawConfig()->RenderScale >= 1 ? 0 : -1);
#endif
	FinalizeBackend();
#if !_COMPILER
	Console = std::make_unique<KHudEngineConsole>();
	Chat = std::make_unique<KHudChatConsole>();
	Menu = std::make_unique<KHudMenu>();
	CharacterInfo = std::make_unique<KHudCharacterInfo>();
	HudScoreboard = std::make_unique<KHudScoreboard>();
	HudTargetName = std::make_unique<KHudTargetName>();
	HudNetStats = std::make_unique<KHudNetStats>();
	HudFrameStats = std::make_unique<KHudFrameStats>();
	HudDamageNumbers = std::make_unique<KHudDamageNumbers>();
	HudOnScreenMessages = std::make_unique<KHudOnScreenMessages>();
	HudPickupMessage = std::make_unique<KHudPickupMessage>();
	HudFragMessage = std::make_unique<KHudFragMessage>();
	HudKillFeed = std::make_unique<KHudKillFeed>();
	HudGameTickStats = std::make_unique<KHudGameFrameStats>();
	HudPowerupTimers = std::make_unique<KHudPowerupTimers>();
	HudLoadScreen = std::make_unique<KHudLoadScreen>();
	HudGameTimer = std::make_unique<KHudGameTimer>();
	crosshair = HUD_LoadImage("crosshairs/default");
	splatter = HUD_LoadImage("splatter");
#endif

	// we are now fully initialized and the game instance will see this
	bInitialized = true;

	// wait for mempools to be created
#if !_COMPILER
	while (!bMemPoolsCreated)
	{
		BufferMutex.lock();
		if (bPendingBufferFunc && CreateBufferFunc) 
		{
			CreateBufferFunc();
			bPendingBufferFunc = false;
		}
		BufferMutex.unlock();
	}
#endif
}

void KRenderInterface::DrawLoop()
{
	KTimePoint LastPresentTime = KTime::Now();
	u64 framecount = 0;
	const auto WaitForLimit = [&]() -> void
	{
#if !_COMPILER
		if (Radiosity.bBuildingRadiosity) return;

		LockConfig();
		i32 fps = i32(GetDrawConfig()->MaxFramerate);
		UnlockConfig();

		//f64 timeSince = KTime::Since(LastPresentTime);
		//f64 f = RoundNearest(timeSince, (1.0 / f64(fps)));
		//if (f < timeSince) f += 1.0 / f64(fps);
		//fps = std::round(1.0 / f);

		while (fps > 0 && KTime::Since(LastPresentTime) < (1.0 / f64(fps))) {}
		LastFrameTime = KTime::Since(LastPresentTime);
		LastPresentTime = KTime::Now();
		framecount++;
		//if (framecount % 100 == 0)
		//{
		//	GetDropConsole()->AddMessage(1.0 / LastFrameTime);
		//}
#endif
	};

	while (!bPendingDestroy) 
	{	
		PrepFrame();
		DrawScene();
		DrawHud();
		WaitForLimit();
		Present();

		FrameStats.FramesSinceReset++;
	}
}

void KRenderInterface::CreateBlendStates()
{
	{
		KBlendData transparency;

		// multiply
		//transparency.SrcBlend = EBlendFactor::BLEND_ZERO;
		//transparency.DestBlend = EBlendFactor::BLEND_SRC_COLOR;
		//transparency.BlendOp = EBlendOp::BLEND_OP_ADD;

		transparency.SrcBlend = EBlendFactor::BLEND_SRC_ALPHA;
		transparency.DestBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
		transparency.BlendOp = EBlendOp::BLEND_OP_ADD;
		transparency.SrcBlendAlpha = EBlendFactor::BLEND_ONE;
		transparency.DestBlendAlpha = EBlendFactor::BLEND_ONE;
		transparency.BlendOpAlpha = EBlendOp::BLEND_OP_ADD;
		GenericTransparency = CreateBlendState(transparency);
	}

	{
		KBlendData transparency;
		transparency.SrcBlend = EBlendFactor::BLEND_SRC_ALPHA;
		transparency.DestBlend = EBlendFactor::BLEND_INV_SRC_ALPHA;
		transparency.BlendOp = EBlendOp::BLEND_OP_ADD;
		transparency.SrcBlendAlpha = EBlendFactor::BLEND_ONE;
		transparency.DestBlendAlpha = EBlendFactor::BLEND_ONE;
		transparency.BlendOpAlpha = EBlendOp::BLEND_OP_ADD;
		

		KBlendData depth;
		depth.SrcBlend = EBlendFactor::BLEND_SRC_COLOR;
		depth.DestBlend = EBlendFactor::BLEND_DEST_COLOR;
		depth.BlendOp = EBlendOp::BLEND_OP_MIN;
		depth.SrcBlendAlpha = EBlendFactor::BLEND_ONE;
		depth.DestBlendAlpha = EBlendFactor::BLEND_ONE;
		depth.BlendOpAlpha = EBlendOp::BLEND_OP_ADD;

		KBlendData arr[2] = { transparency, depth };
		WaterBlendState = CreateBlendState(arr, 2);
	}
}

void KRenderInterface::CreateAndSetGameWindow(u32 resX, u32 resY, u8 state)
{
	Window = CreateGameWindow(resX, resY, state);
}

void KRenderInterface::NotifyWidgetsResize()
{
#if !_COMPILER
	for (KHudWidget* w : Widgets) w->OnWindowResize();
#endif
}	

void KRenderInterface::InitConstantBuffers()
{
	ModelViewProjectionBuffer = CreateConstantBufferDynamic(sizeof(glm::mat4), CB_MVP, EShaderStage::Vertex);
	ModelTransformBuffer = CreateConstantBufferDynamic(sizeof(glm::mat4), CB_MODEL, EShaderStage::Vertex);
	LightMatrixBuffer = CreateConstantBufferDynamic(sizeof(glm::mat4), CB_LIGHTMAT, EShaderStage::Vertex);
	FrameDataBuffer = CreateConstantBufferDynamic(sizeof(f32) * 32, CB_FRAME, EShaderStage::All); // time, cam pitch
	ViewBuffer = CreateConstantBufferDynamic(sizeof(glm::mat4) * 3, CB_VIEW, EShaderStage::Vertex);
	CameraDataBuffer = CreateConstantBufferDynamic(sizeof(KRenderCamera::CameraData), CB_CAMERA, EShaderStage::Pipeline);
	FogBuffer = CreateConstantBufferStatic(sizeof(KFogArray), CB_FOG, EShaderStage::Pixel);

	// bind buffers
	BindBuffer(ModelViewProjectionBuffer.get());
	BindBuffer(ModelTransformBuffer.get());
	BindBuffer(LightMatrixBuffer.get());
	BindBuffer(FrameDataBuffer.get());
	BindBuffer(ViewBuffer.get());
	BindBuffer(CameraDataBuffer.get());
	BindBuffer(FogBuffer.get());

	// init fog to 0
	ClearFog();
}

void KRenderInterface::LoadShaders()
{
	FInputElement position("POSITION", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement texcoord("TEXCOORD", EInputElementType::Float, 2, EInputSlotClass::PerVertex);
	FInputElement normal("NORMAL", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement lightcoord("LIGHTCOORD", EInputElementType::Float, 2, EInputSlotClass::PerVertex);
	FInputElement coordclamp("COORDCLAMP", EInputElementType::Float, 4, EInputSlotClass::PerVertex);
	FInputElement lightcolor("LIGHTCOLOR", EInputElementType::Float, 3, EInputSlotClass::PerVertex);
	FInputElement bufferindex("BUFFERINDEX", EInputElementType::UInt32, 1, EInputSlotClass::PerVertex);

	{
		Shaders.StaticMesh = CreateShaderProgram();
		Shaders.StaticMeshScaledOutline = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "test_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);
		params.InputLayout.Elements.push_back(normal);

		FInputElement prev0("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);
		FInputElement prev1("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 1);
		FInputElement prev2("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 2);
		FInputElement prev3("PREV", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 3);
		FInputElement current0("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);
		FInputElement current1("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 1);
		FInputElement current2("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 2);
		FInputElement current3("CURRENT", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 3);
		FInputElement data("DATA", EInputElementType::Float, 4, EInputSlotClass::PerInstance, 0);

		params.InputLayout.Elements.push_back(prev0);
		params.InputLayout.Elements.push_back(prev1);
		params.InputLayout.Elements.push_back(prev2);
		params.InputLayout.Elements.push_back(prev3);
		params.InputLayout.Elements.push_back(current0);
		params.InputLayout.Elements.push_back(current1);
		params.InputLayout.Elements.push_back(current2);
		params.InputLayout.Elements.push_back(current3);
		params.InputLayout.Elements.push_back(data);

		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.VertexShader.FileName = "static_mesh_vs";
		params.PixelShader.FileName = "static_mesh_ps";
		Shaders.StaticMesh->CreateShadersFromParams(params);
		params.VertexShader.FileName = "static_mesh_scaled_outline_vs";
		params.PixelShader.FileName = "static_mesh_scaled_outline_ps";
		Shaders.StaticMeshScaledOutline->CreateShadersFromParams(params);

		params.VertexShader.FileName = "brain_powerup_vs";
		params.PixelShader.FileName = "brain_powerup_ps";
		Shaders.BrainPowerup = CreateShaderProgram();
		Shaders.BrainPowerup->CreateShadersFromParams(params);

		params.VertexShader.FileName = "health_crystal_vs";
		params.PixelShader.FileName = "health_crystal_ps";
		Shaders.HealthCrystal = CreateShaderProgram();
		Shaders.HealthCrystal->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterEffects = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "water_effects_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.WaterEffects->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterEffectsCS = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.FileName = "water_effects_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.WaterEffectsCS->CreateShadersFromParams(params);
	}

	{
		Shaders.DepthDownsample = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "depth_downsample_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.DepthDownsample->CreateShadersFromParams(params);
	}

	{
		Shaders.FinalProcessing = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "final_effects_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.FinalProcessing->CreateShadersFromParams(params);
	}

	{
		Shaders.BlurCreate = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "blur_create_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BlurCreate->CreateShadersFromParams(params);
	}

	{
		Shaders.BlurApply = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "blur_apply_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BlurApply->CreateShadersFromParams(params);
	}

	{
		Shaders.TextureToScreen = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "texture_to_screen_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.TextureToScreen->CreateShadersFromParams(params);
	}

	{
		Shaders.Radiosity = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.PixelShader.FileName = "radiosity_copy_ps";

		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.Radiosity->CreateShadersFromParams(params);
	}

	{
		Shaders.DebugLine = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "debug_line_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "debug_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.DebugLine->CreateShadersFromParams(params);
	}

	{
		Shaders.DoomSprite = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "doomsprite_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "doomsprite_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.DoomSprite->CreateShadersFromParams(params);
	}

	{
		Shaders.SmokeBeam = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "smoke_beam_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "smoke_dynamic_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.SmokeBeam->CreateShadersFromParams(params);
	}

	{
		Shaders.HitSpark = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "hit_spark_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.HitSpark->CreateShadersFromParams(params);
	}

	{
		Shaders.BlasterParticle = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "blaster_particle_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BlasterParticle->CreateShadersFromParams(params);
	}

	{
		Shaders.BlasterExplosion = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "blaster_explosion_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BlasterExplosion->CreateShadersFromParams(params);
	}

	{
		Shaders.WaterSplash = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "water_splash_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.WaterSplash->CreateShadersFromParams(params);
	}

	{
		Shaders.PortalTravel = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "portal_travel_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.PortalTravel->CreateShadersFromParams(params);
	}

	{
		Shaders.SmokeSheet = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "smoke_sheet_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "smoke_sheet_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.SmokeSheet->CreateShadersFromParams(params);
	}

	{
		Shaders.Explosion = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "explosion_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_noclip_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.Explosion->CreateShadersFromParams(params);
	}

	{
		Shaders.RocketTrail = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "rocket_trail_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "smoke_dynamic_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.RocketTrail->CreateShadersFromParams(params);
	}

	{
		Shaders.Cube = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "bounding_box_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "bounding_box_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.Cube->CreateShadersFromParams(params);
	}

	{
		Shaders.ShotgunShard = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "shotgun_shard_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "shotgun_shard_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.ShotgunShard->CreateShadersFromParams(params);
	}

	{
		Shaders.BulletHole = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "bullet_hole_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "bullet_hole_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BulletHole->CreateShadersFromParams(params);
	}

	{
		Shaders.BloodTrail = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "blood_trail_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BloodTrail->CreateShadersFromParams(params);
	}

	{
		Shaders.TorchFlame = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "torch_flame_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_noclip_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.TorchFlame->CreateShadersFromParams(params);
	}

	{
		Shaders.AtomProjectile = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "atom_projectile_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.AtomProjectile->CreateShadersFromParams(params);
	}

	{
		Shaders.BloodTrail_UnderWater = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "blood_trail_underwater_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "particle_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.BloodTrail_UnderWater->CreateShadersFromParams(params);
	}

	{
		Shaders.Snow = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "snow_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "snow_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.Snow->CreateShadersFromParams(params);
	}
	
	{
		Shaders.LightDepth = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "light_depth_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);

		params.PixelShader.FileName = "light_depth_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LightDepth->CreateShadersFromParams(params);
	}

	{
		Shaders.WorldLit = CreateShaderProgram();
		Shaders.WorldWireframe = CreateShaderProgram();
		Shaders.WorldUnlit = CreateShaderProgram();
		Shaders.WorldNormal = CreateShaderProgram();
		Shaders.WorldNormalTex = CreateShaderProgram();
		Shaders.WorldLightmap = CreateShaderProgram();
		Shaders.Water = CreateShaderProgram();
		Shaders.Portal = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "test_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.InputLayout.Elements.push_back(position);
		params.InputLayout.Elements.push_back(texcoord);
		params.InputLayout.Elements.push_back(normal);
		params.InputLayout.Elements.push_back(lightcoord);
		params.InputLayout.Elements.push_back(coordclamp);
		params.InputLayout.Elements.push_back(lightcolor);
		params.InputLayout.Elements.push_back(bufferindex);
		
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "test_lightmapped_ps";
		Shaders.WorldLit->CreateShadersFromParams(params);

		params.PixelShader.FileName = "test_unlit_ps";
		Shaders.WorldUnlit->CreateShadersFromParams(params);

		params.PixelShader.FileName = "test_wireframe_ps";
		Shaders.WorldWireframe->CreateShadersFromParams(params);

		params.PixelShader.FileName = "test_norm_ps";
		Shaders.WorldNormal->CreateShadersFromParams(params);

		params.PixelShader.FileName = "test_norm_tex_ps";
		Shaders.WorldNormalTex->CreateShadersFromParams(params);

		params.PixelShader.FileName = "world_lightmap_ps";
		Shaders.WorldLightmap->CreateShadersFromParams(params);

		params.PixelShader.FileName = "test_portal_ps";
		Shaders.Portal->CreateShadersFromParams(params);

		params.PixelShader.FileName = "water_ps";
		Shaders.Water->CreateShadersFromParams(params);

		Shaders.ActiveWorld = Shaders.WorldLit.get();
	}

	{
		Shaders.LeakLine = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "leak_line_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.InputLayout.Elements.push_back(position);

		params.PixelShader.FileName = "leak_line_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LeakLine->CreateShadersFromParams(params);
	}

	{
		Program7 = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "test_portal_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.InputLayout.Elements.push_back(position);

		params.PixelShader.FileName = "test_portal_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Program7->CreateShadersFromParams(params);
	}

	{
		Shaders.TextureDownscale = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.FileName = "texture_downscale_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.TextureDownscale->CreateShadersFromParams(params);
	}

	{
		Shaders.VerticalBlur = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.FileName = "blur_vertical_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.VerticalBlur->CreateShadersFromParams(params);
	}

	{
		Shaders.HorizontalBlur = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.FileName = "blur_horizontal_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.HorizontalBlur->CreateShadersFromParams(params);
	}

	{
		Shaders.LightGBufferCS = CreateShaderProgram();
		FShaderProgramCreateParams params;


		params.ComputeShader.FileName = "light_gbuffer_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LightGBufferCS->CreateShadersFromParams(params);
	}

	{
		Shaders.LightCompositeCS = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.ComputeShader.FileName = "light_composite_cs";
		params.ComputeShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LightCompositeCS->CreateShadersFromParams(params);
	}

	{
		Shaders.LightGBuffer = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "light_gbuffer_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LightGBuffer->CreateShadersFromParams(params);
	}

	{
		Shaders.LightComposite = CreateShaderProgram();
		FShaderProgramCreateParams params;

		params.VertexShader.FileName = "fullscreen_quad_vs";
		params.VertexShader.LoadMethod = EShaderLoadMethod::FromFile;

		params.PixelShader.FileName = "light_composite_ps";
		params.PixelShader.LoadMethod = EShaderLoadMethod::FromFile;

		Shaders.LightComposite->CreateShadersFromParams(params);
	}
}

void KRenderInterface::CreateMaterials()
{
	{
		Materials.StandardWorld = std::make_unique<KMaterial>();
		Materials.StandardWorld->Shader = Shaders.WorldLit.get();
		Materials.StandardWorld->CullMode = ECullMode::CullBack;
		Materials.StandardWorld->bTransparent = false;
		Materials.StandardWorld->bWriteDepth = true;
	}
	
	{
		Materials.StandardWater = std::make_unique<KMaterial>();
		Materials.StandardWater->MaterialName = "water";

/*
#if !_COMPILER
		const auto bindWaterTargets = [&]() -> void
		{
			KRenderTarget* targets[2] =
			{
				WaterTarget.get(),
				WaterDepthTarget.get()
			};
			BindRenderTargets(targets, 2, SceneDepthBuffer.get());
		};
		Materials.StandardWater->ExtraBindFunction = bindWaterTargets;
#endif

		Materials.StandardWater->Shader = Shaders.Water.get();
		Materials.StandardWater->CullMode = ECullMode::CullNone;
		Materials.StandardWater->bTransparent = false;
		Materials.StandardWater->bWriteDepth = false;
		Materials.StandardWater->bTransparentPass = true;*/

		// water gets its own special case in KRenderScene so it does not need to be bound via material
	}

	{
		Materials.StandardPortal = std::make_unique<KMaterial>();
		Materials.StandardPortal->MaterialName = "portal";
		Materials.StandardPortal->Shader = Shaders.Portal.get();
		Materials.StandardPortal->CullMode = ECullMode::CullNone;
		Materials.StandardPortal->bTransparent = false;
		Materials.StandardPortal->bWriteDepth = true;
	}

	Materials.NameToMaterial = 
	{
		{ "", Materials.StandardWorld.get() },
		{ "water", Materials.StandardWater.get() },
		{ "portal", Materials.StandardPortal.get() },
	};
}

void KRenderInterface::SetWorldShader(u8 index)
{
	Rasterizer->SetFillMode(EFillMode::Solid);
	Rasterizer->SetCullMode(ECullMode::CullBack);
	ERenderMode mode = ERenderMode(index);
	Shaders.WorldRenderMode = index;

	switch(mode)
	{
		case ERenderMode::Lit:
			//Shaders.ActiveWorld = Shaders.WorldLit.get();
			Materials.StandardWorld->Shader = Shaders.WorldLit.get();
			break;
		case ERenderMode::Unlit:
			//Shaders.ActiveWorld = Shaders.WorldUnlit.get();
			Materials.StandardWorld->Shader = Shaders.WorldUnlit.get();
			break;
		case ERenderMode::Wireframe:
			Rasterizer->SetFillMode(EFillMode::Wireframe);
			Rasterizer->SetCullMode(ECullMode::CullNone);
			//Shaders.ActiveWorld = Shaders.WorldWireframe.get();
			Materials.StandardWorld->Shader = Shaders.WorldWireframe.get();
			break;
		case ERenderMode::Normal:
			//Shaders.ActiveWorld = Shaders.WorldNormal.get();
			Materials.StandardWorld->Shader = Shaders.WorldNormal.get();
			break;
		case ERenderMode::NormalTex:
			//Shaders.ActiveWorld = Shaders.WorldNormalTex.get();
			Materials.StandardWorld->Shader = Shaders.WorldNormalTex.get();
			break;
		case ERenderMode::Lightmapped:
			//Shaders.ActiveWorld = Shaders.WorldLightmap.get();
			Materials.StandardWorld->Shader = Shaders.WorldLightmap.get();
			break;
	}
}

void KRenderInterface::OnNewMapLoaded()
{
	/*WorldScene.reset();
	WorldScene = std::make_unique<KRenderScene>();
	WorldScene->SubmitFaces(KMapCompiler::Get().GetWorldSpawn()->Faces);

	if (KMapEntity* sky = KMapCompiler::Get().GetTempSky())
	{
		SkyboxScene.reset();
		SkyboxScene = std::make_unique<KRenderScene>();
		SkyboxScene->SubmitFaces(sky->Faces);
	}*/
}

void KRenderInterface::BindGBuffer()
{
	KRenderTarget* buffs[] = { GBuffer.Color.get(), GBuffer.Position.get(), GBuffer.Normal.get() };
	BindRenderTargets(buffs, 3, SceneDepthBuffer.get());
}

void KRenderInterface::LightOpaque()
{return;
if (false)//(((u32)KTime::SinceInit() / 3) % 2 == 0)
{
	KRenderTarget* nulls[] = { nullptr, nullptr, nullptr };
	BindRenderTargets(nulls, 3);
	GBuffer.Color->BindTexture2D(0, EShaderStage::Compute);
	GBuffer.Position->BindTexture2D(1, EShaderStage::Compute);
	GBuffer.Normal->BindTexture2D(2, EShaderStage::Compute);
	SetUAVForCS(LightCompositeTarget.get());
	BindShaderProgram(Shaders.LightGBufferCS.get());
	Dispatch(GetRenderResX() / 16, GetRenderResY() / 16, 1);
	BindTexture2D(0, 0, EShaderStage::Compute);
	BindTexture2D(0, 1, EShaderStage::Compute);
	BindTexture2D(0, 2, EShaderStage::Compute);
	SetUAVForCS(nullptr);

	// composite
	GBuffer.Color->BindTexture2D(0, EShaderStage::Compute);
	LightCompositeTarget->BindTexture2D(1, EShaderStage::Compute);	
	SetUAVForCS(SolidTarget.get());
	BindShaderProgram(Shaders.LightCompositeCS.get());
	Dispatch(GetRenderResX() / 8, GetRenderResY() / 8, 1);
}
else
{
	Rasterizer->SetCullMode(ECullMode::CullNone);
	Rasterizer->SetFillMode(EFillMode::Solid);

	// clear GBuffer
	KRenderTarget* nulls[] = { nullptr, nullptr, nullptr };
	BindRenderTargets(nulls, 3);

	SetViewport(GetRenderResX() / 2, GetRenderResY() / 2);

	GBuffer.Color->BindTexture2D(0);
	GBuffer.Position->BindTexture2D(1);
	GBuffer.Normal->BindTexture2D(2);
	BindRenderTarget(LightCompositeTarget.get());
	BindShaderProgram(Shaders.LightGBuffer.get());
	Draw(3, 0);
	//BindTexture2D(0, 0);
	BindTexture2D(0, 1);
	BindTexture2D(0, 2);
	BindRenderTarget(nullptr);
	SetViewport(GetRenderResX(), GetRenderResY());

	// composite
	//GBuffer.Color->BindTexture2D(0);
	LightCompositeTarget->BindTexture2D(1);
	BindRenderTarget(SolidTarget.get());
	BindShaderProgram(Shaders.LightComposite.get());
	Draw(3, 0);
}
}

void KRenderInterface::ProcessPendingScreenshot()
{
	if (bPendingScreenshot)
	{
		Rasterizer->SetFillMode(EFillMode::Solid);
		Rasterizer->SetCullMode(ECullMode::CullNone);
		BindShaderProgram(Shaders.FinalProcessing.get());
		SetViewport(GetViewportX(), GetViewportY());
		BindVertexBuffer(nullptr);
		FinalTarget->BindTexture2D(0);
		FRenderTargetCreationFlags flags;
		UPtr<KRenderTarget> tar = CreateRenderTarget(GetViewportX(), GetViewportY(), flags);

		BindRenderTarget(tar.get(), nullptr);
		Draw(3, 0);
		tar->Screenshot();

		bPendingScreenshot = false;
		BindTexture2D(0, 0);
	}
}

void KRenderInterface::UpdatePerFrameData()
{
	struct
	{
		f32 Time;
		f32 Pitch;
		f32 RenderAlpha;
		f32 pad[1];
		f32 ResX;
		f32 ResY;
		f32 pad1[2];
		u32 ActiveFlashCount = 0;
		u32 pad2[3];
		u32 ActiveLightCount = 0;
		u32 pad3[3];
		f32 LastTeleportTime = -1;
		f32 pad4[3];
		f32 UnderWaterDepth = 0;
		f32 pad5[3];
		f32 RenderScale;
		f32 pad6[3];
		f32 Saturation = 1;
		f32 Brightness = 1;
		f32 Contrast = 1;
		f32 Flip = 0;
	} data;
	TotalRenderTime += LastFrameTime * GameTimeDilation;
	data.Time = TotalRenderTime;
	data.Pitch = Camera.LastRecordedPitch * (180.f / PI<f32>());
	data.RenderAlpha = RenderAlpha;
	data.ResX = GetRenderResX();
	data.ResY = GetRenderResY();
	data.LastTeleportTime = LastTeleportTime;
	data.UnderWaterDepth = UnderWaterDepth;
	data.RenderScale = GetRenderResX() / GetViewportX();
	data.Saturation = GetDrawConfig()->Saturation;
	data.Contrast = GetDrawConfig()->Contrast;
	data.Flip = GetDrawConfig()->Flip;

	f32 b = GetDrawConfig()->Brightness;
	if (b < 1)
		b = MapRange(b, 0, 1, .8, 1);
	else if (b > 1)
		b = MapRange(b, 1, 2, 1, 1.5);
	data.Brightness = b;
#if !_COMPILER
	data.ActiveFlashCount = GetInfoFromRender(&typeid(KLightFlash)).ActiveCount;
	data.ActiveLightCount = GetInfoFromRender(&typeid(KDynamicLight)).ActiveCount;
#else
	data.ActiveFlashCount = 0;
	data.ActiveLightCount = 0;
#endif
	UpdateBuffer(FrameDataBuffer.get(), &data);
}

void KRenderInterface::TakeScreenshot()
{
	//FinalTarget->Screenshot();
	bPendingScreenshot = true;
}

void KRenderInterface::UpdateFog(const TVector<KFogBuffer>& fog)
{
	KFogArray arr;
	arr.FogCount = fog.size();
	memcpy(arr.Fog, fog.data(), sizeof(KFogBuffer) * arr.FogCount);
	UpdateBuffer(FogBuffer.get(), &arr);
}

void KRenderInterface::ClearFog()
{
	KFogArray arr;
	arr.FogCount = 0;
	UpdateBuffer(FogBuffer.get(), &arr);
}

void KRenderInterface::LoadFonts()
{
#if !_DEV
	HUD_LoadFontsFromWad();
#endif

	// load fonts to fit the current window
	HUD_LoadFont("Chakra Petch", 20, EFontUsage::Console);
	HUD_LoadFont("Falling Sky", 16, EFontUsage::Chat);
	HUD_LoadFont("Falling Sky", 18, EFontUsage::Scoreboard);
	HUD_LoadFont("Arial Monospaced MT", 16, EFontUsage::Stats, true);
	HUD_LoadFont("Red October", 45, EFontUsage::PowerupTimer, true);
	HUD_LoadFont("Red October", 35, EFontUsage::GameTimer, true);
	HUD_LoadFont("Red October", 20, EFontUsage::DamageNumber, true);
	HUD_LoadFont("Falling Sky", 45, EFontUsage::LoadScreen);
	HUD_LoadFont("Vicious Hunger Rotalic", 65, EFontUsage::MainLogo);
	HUD_LoadFont("Vicious Hunger", 40, EFontUsage::MenuButtonLarge);
	HUD_LoadFont("Vicious Hunger", 30, EFontUsage::MenuButtonSmall);
	HUD_LoadFont("Chakra Petch", 25, EFontUsage::FragMessage, true);
	HUD_LoadFont("Chakra Petch", 30, EFontUsage::DeathMessage, true);
	HUD_LoadFont("Chakra Petch", 20, EFontUsage::OnScreenMessage, true);
	HUD_LoadFont("Chakra Petch", 20, EFontUsage::KillFeed);
}

void KRenderInterface::AllocateScenes()
{
	WorldScene.reset();
	SkyboxScene.reset();
	WorldScene = std::make_unique<KRenderScene>();
	SkyboxScene = std::make_unique<KRenderScene>();
}

class KRenderScene* KRenderInterface::GetWorldScene()
{
	return WorldScene.get();
}

class KRenderScene* KRenderInterface::GetSkyboxScene()
{
	return SkyboxScene.get();
}

void KRenderInterface::EnableTransparency()
{
	BindBlendState(GenericTransparency.get());
	//BindRenderTarget(AccumTarget.get(), nullptr);
	//SetViewport(GetRenderResX() / 2, GetRenderResY() / 2);
	//SceneDepthBuffer->BindTexture2D(5);
}

void KRenderInterface::DisableTransparency()
{
	ClearBlendState();
	//BindTexture2D(0, 5);
	//BindRenderTarget(PostTarget.get(), SceneDepthBuffer.get());
	//SetViewport(GetRenderResX(), GetRenderResY());
}

void KRenderInterface::EnableWaterBlend()
{
	BindBlendState(WaterBlendState.get());
}

void KRenderInterface::DisableWaterBlend()
{
	ClearBlendState();
}

void KRenderInterface::ResizeResolution(f32 x, f32 y)
{
	glfwSetWindowSize(GetGameWindow()->GetGlfwWindow(), x, y);
}

KDrawConfig* KRenderInterface::GetDrawConfig() const
{
#if _COMPILER
	return nullptr;
#else
	return GetUserConfig()->GetDrawConfig();
#endif
}

/*
class KTexture2D* KRenderInterface::GetTexture2D(const KString& name)
{
	if (LoadedTexture2D.contains(name)) return LoadedTexture2D[name].get();
	return nullptr;
}*/

void KRenderInterface::BindShaderProgram(class KShaderProgram* program)
{
	program->BindProgram();
}

UPtr<KTexture2D> KRenderInterface::CreateTexture2DFromSurface(UPtr<class KSurface2D> surface)
{
	//if (LoadedTexture2D.contains(name)) return nullptr;

	auto tex = CreateTexture2D();

	// moves the surface pointer and is no longer valid here
	tex->CreateFromSurface(surface);

	//KTexture2D* t = tex.get();
	//LoadedTexture2D[name] = std::move(tex);
	return tex;
}

UPtr<KTexture2D> KRenderInterface::CreateTexture2DFromFreetypeGlyph(struct FT_GlyphSlotRec_* glyph)
{
	auto tex = CreateTexture2D();
	tex->CreateFromFreetypeGlyph(glyph);
	return tex;
}

UPtr<KTexture2D> KRenderInterface::LoadTexture2D(const class KString& name)
{
	// if we already have this texture, skip it
	//if (LoadedTexture2D.contains(name)) return LoadedTexture2D[name].get();

	auto surface = std::make_unique<KSurface2D>();
	surface->LoadPNG(name);	
#if _PACK
	KGameInstance::Get().MainWadFile.AddImage(surface.get());
#endif
	return CreateTexture2DFromSurface(std::move(surface));
}

UPtr<class KVertexBuffer> KRenderInterface::CreateParticleIndexBuffer(u32 count)
{
	u32 ind[6] = { 0, 2, 1, 2, 3, 1 };
	TVector<u32> indices;
	indices.resize(6 * count);
	for (u32 i = 0; i < count * 6; i++)
	{
		u32 index = i % 6;
		u32 particle = i / 6;
		indices[i] = ind[index] + particle * 4;
	}
	return CreateVertexBufferStatic(nullptr, 0, 0, indices);
}

void KRenderInterface::EnableDepthBuffer()
{
	K_ASSERT(SceneDepthBuffer.get(), "cannot enable null depth buffer");
	SceneDepthBuffer->SetEnabled(true);
}

void KRenderInterface::DisableDepthBuffer()
{
	K_ASSERT(SceneDepthBuffer.get(), "cannot disable null depth buffer");
	SceneDepthBuffer->SetEnabled(false);
}

void KRenderInterface::SetDepthBufferReadOnly(bool read_only)
{
	K_ASSERT(SceneDepthBuffer.get(), "cannot set read write state of null depth buffer");
	SceneDepthBuffer->SetReadOnly(read_only);
}

void KRenderInterface::ClearDepthBuffer()
{
	SceneDepthBuffer->Clear();
}

void KRenderInterface::UpdateBuffer(class KGpuBuffer* buffer, const void* data, u32 elemCount /*= 0*/)
{
	K_ASSERT(buffer, "KGpuBuffer to be updated was nullptr");
	buffer->Update(data, elemCount);
}

void KRenderInterface::BlurRenderTarget(KRenderTarget* target, bool apply /*= true*/)
{
	BindRenderTarget(BlurOutputTarget.get());
	target->BindTexture2D(0);
	BindShaderProgram(Shaders.BlurCreate.get());
	BindVertexBuffer(nullptr);
	Draw(3, 0);

	if (apply)
	{
		BindTexture2D(0, 0);
		BindRenderTarget(target);
		BlurOutputTarget->BindTexture2D(0);
		BindShaderProgram(Shaders.BlurApply.get());
		Draw(3, 0);
	}
}

void KRenderInterface::CreateDownsampleDepthBuffer()
{
	ClearBlendState();
	//SetViewport(GetRenderResX() / 2, GetRenderResY() / 2);
	SceneDepthBuffer_Downsampled->SetEnabled(true);
	SceneDepthBuffer_Downsampled->SetReadOnly(false);
	SceneDepthBuffer_Downsampled->Clear();
	BindRenderTarget(nullptr, SceneDepthBuffer_Downsampled.get());
	SceneDepthBuffer->BindTexture2D(0);
	BindShaderProgram(Shaders.DepthDownsample.get());
	Draw(3, 0);
	BindTexture2D(nullptr, 0);

}

void KRenderInterface::CreateRenderTargets()
{
	{
		FinalTarget.reset();
		FRenderTargetCreationFlags f;
		f.EnableUnorderedAccess();
		FinalTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
	}

	/*{
		BlurOutputTarget.reset();
		FRenderTargetCreationFlags f;
		BlurOutputTarget = CreateRenderTarget(GetRenderResX() / 8, GetRenderResY() / 8, f);
	}*/

	{
		SolidTarget.reset();
		BlurOutputTarget.reset();
		BrightPixelsTarget.reset();
		LightCompositeTarget.reset();
		FRenderTargetCreationFlags f;
		//BrightPixelsTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
		f.EnableUnorderedAccess();
		SolidTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
		FRenderTargetCreationFlags ff;
		ff.Enable32Bit();
		LightCompositeTarget = CreateRenderTarget(GetRenderResX() / 2, GetRenderResY() / 2, ff);
		//BlurOutputTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
	}
	
	{
		WaterTarget.reset();
		FRenderTargetCreationFlags f;
		WaterTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
	}
	
	{
		WaterDepthTarget.reset();
		FRenderTargetCreationFlags f;
		f.Enable32Bit();
		WaterDepthTarget = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
	}

	// TODO render transparency (and GBuffer?) at half res or smaller

	{
		TransparencyTarget.reset();
		FRenderTargetCreationFlags f;
		TransparencyTarget = CreateRenderTarget(GetRenderResX() / 2, GetRenderResY() / 2, f);
	}

	{
		GBuffer.Position.reset();
		GBuffer.Normal.reset();
		GBuffer.Color.reset();
		FRenderTargetCreationFlags f;
		GBuffer.Color = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
		f.Enable32Bit();
		GBuffer.Position = CreateRenderTarget(GetRenderResX(), GetRenderResY(), f);
		FRenderTargetCreationFlags ff;
		ff.EnableSignedNorm();
		GBuffer.Normal = CreateRenderTarget(GetRenderResX(), GetRenderResY(), ff);
	}
}

void KRenderInterface::AddPrecipitation(const KPrecipitationVolume& p)
{
	if (WorldScene) WorldScene->AddPrecipitation(p);
}

void KRenderInterface::CallInterfaceDestruction()
{
	bPendingDestroy = true;
	if (HasRenderThread()) RenderThread.join();
}

void KRenderInterface::BindWorldShader()
{
	SetWorldShader(Shaders.WorldRenderMode);
	BindShaderProgram(Shaders.ActiveWorld);
}

void KRenderInterface::SetModelViewProjection(const glm::mat4& mvp)
{
	UpdateBuffer(ModelViewProjectionBuffer.get(), &mvp);
	ActiveMVP = mvp;
}

void KRenderInterface::SetModelTransform(const glm::mat4 model)
{
	UpdateBuffer(ModelTransformBuffer.get(), &model);
}

void KRenderInterface::SetMvpFromModel(const glm::mat4 model)
{
	SetModelTransform(model);
	SetModelViewProjection(glm::transpose(Camera.GetViewProjectionMatrix() * model));
}

void KRenderInterface::ProcessCommand(u64 command)
{
	// render bridge is locked right now
	const KPendingCommands& pending = KRenderBridge::Get().GetPendingCommands();

	EPendingCommandType comm = EPendingCommandType(command);
	switch (comm)
	{
		case EPendingCommandType::RenderMode:
		{
			SetWorldShader(pending.RenderMode);
			return;
		}
		case EPendingCommandType::FOV:
		{
			
			return;
		}
		case EPendingCommandType::Resolution:
		{
			
			return;
		}
		case EPendingCommandType::Screenshot:
		{
			TakeScreenshot();
			return;
		}
	}
}

glm::vec2 KRenderInterface::ProjectWorldToScreen(const glm::vec3& worldPoint)
{
	glm::vec4 clipSpace = Camera.ProjectionMatrix * (Camera.ViewMatrix * glm::vec4(worldPoint, 1.f));
	glm::vec3 ndc = glm::vec3(clipSpace.x, -clipSpace.y, clipSpace.z) / clipSpace.w;
	return ((glm::vec2(ndc.x, ndc.y) + 1.f) / 2.f) * glm::vec2(GetViewportX(), GetViewportY());
}

bool KRenderInterface::IsShowingScoreboard() const
{
#if _COMPILER
	return false;
#else
	return HudScoreboard && HudScoreboard->bShowing;
#endif
}
DVec3 KRenderInterface::CalculateRadiosityAtPoint(const DVec3& point, const DVec3& norm, class KBrushFace* face)
{
	static std::mutex radiosityMutex;
	radiosityMutex.lock();

	if (!Radiosity.Front.get())
	{
		//Radiosity.Depth = CreateDepthBuffer(EDepthUsage::SceneDepth, GI_RESOLUTION, GI_RESOLUTION);
		FRenderTargetCreationFlags flags;
		Radiosity.Front = CreateRenderTarget(GI_RESOLUTION, GI_RESOLUTION, flags);
		Radiosity.Up = CreateRenderTarget(GI_RESOLUTION, GI_RESOLUTION, flags);
		Radiosity.Down = CreateRenderTarget(GI_RESOLUTION, GI_RESOLUTION, flags);
		Radiosity.Right = CreateRenderTarget(GI_RESOLUTION, GI_RESOLUTION, flags);
		Radiosity.Left = CreateRenderTarget(GI_RESOLUTION, GI_RESOLUTION, flags);
		Radiosity.FinalTarget = CreateRenderTarget(GI_RESOLUTION * 2, GI_RESOLUTION * 2, flags);
	}

	Radiosity.FinalTarget->Clear(0, 0, 0, 1);
	//Radiosity.Depth->Clear();
	SceneDepthBuffer->Clear();
	Radiosity.Front->Clear(0, 0, 0, 1);
	Radiosity.Up->Clear(0, 0, 0, 1);
	Radiosity.Down->Clear(0, 0, 0, 1);
	Radiosity.Right->Clear(0, 0, 0, 1);
	Radiosity.Left->Clear(0, 0, 0, 1);

	enum
	{
		Direction_Forward,
		Direction_Up,
		Direction_Down,
		Direction_Right,
		Direction_Left,
	};

	const auto renderCubeFace = [&](const DVec3& point, const DVec3& normal, const DVec3& up, u8 dir) -> void
	{
		KDepthBuffer* depth = SceneDepthBuffer.get();
		KRenderTarget* finalTarget = nullptr;
		depth->Clear();
		switch (dir)
		{
			case Direction_Forward:
				finalTarget = Radiosity.Front.get();
				break;
			case Direction_Up:
				finalTarget = Radiosity.Up.get();
				break;
			case Direction_Down:
				finalTarget = Radiosity.Down.get();
				break;
			case Direction_Left:
				finalTarget = Radiosity.Left.get();
				break;
			case Direction_Right:
				finalTarget = Radiosity.Right.get();
				break;
		}

		WaterTarget->Clear(0, 0, 0, 0);
		WaterDepthTarget->Clear(1, 999999, 0, 0);
		TransparencyTarget->Clear(0, 0, 0, 0);
		SolidTarget->Clear(0, 0, 0, 0);

		if (SkyboxScene)
		{
			Camera.SetRadiosityPerspective(SkyboxScene->GetBounds().GetCenter().ToGLM(), normal.ToGLM(), up.ToGLM(), GI_RESOLUTION);
			SkyboxScene->DrawOpaque(true, finalTarget);
			depth->Clear();
		}
		if (face->Surface & ESurfaceFlags::GI_SKY_ONLY) return;
		Camera.SetRadiosityPerspective(point.ToGLM(), normal.ToGLM(), up.ToGLM(), GI_RESOLUTION);
		WorldScene->DrawOpaque(false, finalTarget);
		WorldScene->DrawWater();
	};

	DVec3 rightDir = -norm.Cross
	(
		norm == DVec3(0, 0, 1) ?
		DVec3(1, 0, 0) :
		norm == DVec3(0, 0, -1) ?
		DVec3(-1, 0, 0) :
		DVec3(0, 0, 1)
	);
	DVec3 upDir = norm.Cross(rightDir);
	rightDir.Normalize();
	upDir.Normalize();

	// unbind resources
	BindTexture2D(nullptr, 0, EShaderStage::Pixel);
	BindTexture2D(nullptr, 1, EShaderStage::Pixel);
	BindTexture2D(nullptr, 2, EShaderStage::Pixel);
	BindTexture2D(nullptr, 3, EShaderStage::Pixel);
	BindTexture2D(nullptr, 4, EShaderStage::Pixel);

	SetNullRenderTarget();

	// draw each angle
	SetViewport(GI_RESOLUTION, GI_RESOLUTION);
	renderCubeFace(point, norm, upDir, Direction_Forward);
	renderCubeFace(point, upDir, upDir.Cross(rightDir), Direction_Up);
	renderCubeFace(point, -upDir, norm, Direction_Down);
	renderCubeFace(point, -rightDir, upDir, Direction_Right);
	renderCubeFace(point, rightDir, upDir, Direction_Left);

	// create final image from each render
	SetViewport(GI_RESOLUTION * 2, GI_RESOLUTION * 2);


	BindRenderTarget(Radiosity.FinalTarget.get()); // no depth
	BindShaderProgram(Shaders.Radiosity.get());
	BindVertexBuffer(nullptr);
	Radiosity.Front->BindTexture2D(0);
	Radiosity.Up->BindTexture2D(1);
	Radiosity.Down->BindTexture2D(2);
	Radiosity.Left->BindTexture2D(3);
	Radiosity.Right->BindTexture2D(4);
	Draw(3, 0);

	SetNullRenderTarget();
	BindTexture2D(nullptr, 0);

#if SAVE_GI_SCREENSHOT
	Radiosity.FinalTarget->Screenshot();
#endif

#if COMPILER_SHOW_WINDOW
	ClearBackBuffer();
	Radiosity.FinalTarget->BindTexture2D(0);
	BindShaderProgram(Shaders.TextureToScreen.get());
	BindBackBuffer(false);
	Draw(3, 0);
	Present();
	Sleep(10);
	// prevents windows from freezing the application
	glfwPollEvents();
#endif

	// copy render to cpu
	DVec3 totalColor;
	u32 pixelCount = 0;
	UPtr<KMappedRender> mapped = Radiosity.FinalTarget->GetMapped();

	radiosityMutex.unlock();

	for (u32 x = 0; x < GI_RESOLUTION * 2; x++)
	{
		for (u32 y = 0; y < GI_RESOLUTION * 2; y++)
		{
			// skip corner sections
			u32 q = (GI_RESOLUTION / 4);
			if (x < q || x > GI_RESOLUTION - q)
				if (y < q || y > GI_RESOLUTION - q) continue;

			FColor32 color = mapped->GetAt(x, y).To32();
			totalColor += DVec3(color.r, color.g, color.b);
			pixelCount++;
		}
	}

	/*Radiosity.ColorResult = totalColor / pixelCount;
	Radiosity.bHasResult = true;
	Radiosity.bPendingTest = false;*/
	return totalColor / pixelCount;
}

#endif