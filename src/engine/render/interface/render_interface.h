#pragma once

#if !_SERVER

#include <memory>
#include "kfglobal.h"
#include <thread>
#include <atomic>
#include <mutex>
#include "engine/system/time.h"
#include "../shader_stage.h"
#include "../camera.h"
#include "depth_buffer.h"
#include "../2d_prim.h"
#include "../font_usage.h"
#include "engine/math/vec3.h"
#include "../frame_stats.h"
#include "../render_buffers.h"
#include "../buffers/buffer_fog.h"
#include "../../console/pending_command.h"
#include "../../game/damage_number.h"


class KRenderInterface
{
	friend class KGameInstance;
	friend class KRenderBridge;
	friend class KMapFile;
	friend class KMapCompiler;

public:

	UPtr<class KHudBitmap> crosshair;
	UPtr<class KHudBitmap> splatter;

	// TEMP
	f32 WeaponSwitchAlpha = 0;

	UPtr<class KGpuBuffer> ModelViewProjectionBuffer;
	UPtr<class KGpuBuffer> ModelTransformBuffer;
	UPtr<class KGpuBuffer> ViewBuffer;
	UPtr<class KGpuBuffer> CameraDataBuffer;
	UPtr<class KGpuBuffer> FrameDataBuffer;
	UPtr<class KGpuBuffer> ParticleSystemBuffer; // current bound particle system
	UPtr<class KGpuBuffer> LightMatrixBuffer;
	UPtr<class KGpuBuffer> PositionBuffer;
	UPtr<class KGpuBuffer> FogBuffer;
	UPtr<class KShaderProgram> Program1;
	UPtr<class KShaderProgram> Program2;
	UPtr<class KShaderProgram> LightDepth;
	UPtr<class KShaderProgram> Program5;
	UPtr<class KShaderProgram> Program7;

public:

	struct
	{
		u8 WorldRenderMode = 0;
		UPtr<class KShaderProgram> WorldLit;
		UPtr<class KShaderProgram> WorldUnlit;
		UPtr<class KShaderProgram> WorldWireframe;
		UPtr<class KShaderProgram> WorldNormal;
		UPtr<class KShaderProgram> WorldNormalTex;
		UPtr<class KShaderProgram> WorldLightmap;
		UPtr<class KShaderProgram> LeakLine;

		UPtr<class KShaderProgram> LightDepth;

		UPtr<class KShaderProgram> TextureToScreen;
		UPtr<class KShaderProgram> BlurCreate;
		UPtr<class KShaderProgram> BlurApply;
		UPtr<class KShaderProgram> FinalProcessing;
		UPtr<class KShaderProgram> WaterEffects;
		UPtr<class KShaderProgram> WaterEffectsCS;
		UPtr<class KShaderProgram> DepthDownsample;
		UPtr<class KShaderProgram> DebugLine;
		UPtr<class KShaderProgram> Particle;
		UPtr<class KShaderProgram> SmokeDynamic;
		UPtr<class KShaderProgram> SmokeBeam;
		UPtr<class KShaderProgram> HitSpark;
		UPtr<class KShaderProgram> WaterSplash;
		UPtr<class KShaderProgram> PortalTravel;
		UPtr<class KShaderProgram> Explosion;
		UPtr<class KShaderProgram> RocketTrail;
		UPtr<class KShaderProgram> Cube;
		UPtr<class KShaderProgram> ShotgunShard;
		UPtr<class KShaderProgram> Snow;
		UPtr<class KShaderProgram> Water;
		UPtr<class KShaderProgram> Portal;
		UPtr<class KShaderProgram> StaticMesh;
		UPtr<class KShaderProgram> StaticMeshScaledOutline;
		UPtr<class KShaderProgram> BulletHole;
		UPtr<class KShaderProgram> BloodTrail;
		UPtr<class KShaderProgram> BloodTrail_UnderWater;
		UPtr<class KShaderProgram> TorchFlame;
		UPtr<class KShaderProgram> BrainPowerup;
		UPtr<class KShaderProgram> HealthCrystal;
		UPtr<class KShaderProgram> AtomProjectile;
		UPtr<class KShaderProgram> SmokeSheet;
		UPtr<class KShaderProgram> BlasterParticle;
		UPtr<class KShaderProgram> BlasterExplosion;
		UPtr<class KShaderProgram> LightGBufferCS;
		UPtr<class KShaderProgram> LightCompositeCS;
		UPtr<class KShaderProgram> LightGBuffer;
		UPtr<class KShaderProgram> LightComposite;

		UPtr<class KShaderProgram> DoomSprite;

		UPtr<class KShaderProgram> Radiosity;

		class KShaderProgram* ActiveWorld = nullptr;

		UPtr<class KShaderProgram> TextureDownscale;
		UPtr<class KShaderProgram> VerticalBlur;
		UPtr<class KShaderProgram> HorizontalBlur;

	} Shaders;


	struct 
	{
		UPtr<class KMaterial> StandardWorld;
		UPtr<class KMaterial> StandardWater;
		UPtr<class KMaterial> StandardPortal;

		TMap<KString, class KMaterial*> NameToMaterial;

		KMaterial* GetByName(const KString& name)
		{
			if (NameToMaterial.contains(name))
				return NameToMaterial[name];
			
			return StandardWorld.get();
		}
	} Materials;

private:

	struct
	{
		UPtr<class KRenderTarget> Front;
		UPtr<class KRenderTarget> Up;
		UPtr<class KRenderTarget> Down;
		UPtr<class KRenderTarget> Left;
		UPtr<class KRenderTarget> Right;
		UPtr<class KRenderTarget> FinalTarget; // all renders get pasted on here
		UPtr<class KDepthBuffer> Depth;

		std::atomic<bool> bPendingTest = { false };
		DVec3 PendingPoint;
		DVec3 PendingNorm;
		class KBrushFace* PendingFace = nullptr;

		std::atomic<bool> bHasResult = { false };
		DVec3 ColorResult;

		// true during the entire radiosity process
		std::atomic<bool> bBuildingRadiosity = { false };

	} Radiosity;

	// part of a workaround for inability to have virtual template functions
#if _WIN32
	class KRenderInterface_D3D11* D3D11Interface = nullptr;
#endif
	class KRenderInterface_OpenGL* OpenGLInterface = nullptr;

	// data for the current level
	UPtr<class KRenderScene> WorldScene;
	UPtr<class KRenderScene> SkyboxScene;

public:

	UPtr<class KGameWindow> Window;
	UPtr<class KRasterizerState> Rasterizer;
	UPtr<class KDepthBuffer> SceneDepthBuffer;
	UPtr<class KDepthBuffer> SceneDepthBuffer_Downsampled;

	UPtr<class KBlendState> GenericTransparency;
	UPtr<class KBlendState> WaterBlendState;

	// full screen triangle (interpreted as a quad) for all scene rendering to render to
	UPtr<class KRenderTarget> FinalTarget;


	UPtr<class KRenderTarget> SolidTarget;

	UPtr<class KRenderTarget> WaterTarget;
	UPtr<class KRenderTarget> WaterDepthTarget;
	UPtr<class KRenderTarget> TransparencyTarget;
	UPtr<class KRenderTarget> BlurOutputTarget;
	UPtr<class KRenderTarget> BrightPixelsTarget;
	UPtr<class KRenderTarget> LightCompositeTarget;
	
	struct
	{
		UPtr<class KRenderTarget> Color;
		UPtr<class KRenderTarget> Normal;
		UPtr<class KRenderTarget> Position;
	} GBuffer;


protected:

	TMap<class KString, UPtr<class KTexture2D>> LoadedTexture2D;

	KRenderCamera Camera;

#if !_COMPILER
	// HUD widgets
	UPtr<class KHudEngineConsole> Console;
	UPtr<class KHudChatConsole> Chat;
	UPtr<class KHudMenu> Menu;
	UPtr<class KHudCharacterInfo> CharacterInfo;
	UPtr<class KHudNetStats> HudNetStats;
	UPtr<class KHudTargetName> HudTargetName;
	UPtr<class KHudScoreboard> HudScoreboard;
	UPtr<class KHudFrameStats> HudFrameStats;
	UPtr<class KHudGameFrameStats> HudGameTickStats;
	UPtr<class KHudDamageNumbers> HudDamageNumbers;
	UPtr<class KHudFragMessage> HudFragMessage;
	UPtr<class KHudOnScreenMessages> HudOnScreenMessages;
	UPtr<class KHudPickupMessage> HudPickupMessage;
	UPtr<class KHudKillFeed> HudKillFeed;
	UPtr<class KHudPowerupTimers> HudPowerupTimers;
	UPtr<class KHudLoadScreen> HudLoadScreen;
	UPtr<class KHudGameTimer> HudGameTimer;
public:
	TVector<class KHudWidget*> Widgets;
#endif


protected:

	// the game instance will wait until the renderer is initialized to continue
	// the renderer will be in a different thread when this is flagged true
	std::atomic<bool> bInitialized { false };

private:

	std::atomic<bool> bDrawStats = { true };

	// this thread contains the render loop
	// rendering is continuous and separate from the game loop
	std::thread RenderThread;

	// used to notify the render thread we are being destroyed
	std::atomic<bool> bPendingDestroy { false };
	
	KTimePoint LastPresentTime;

	bool bRenderThread = true;

	KTimePoint LastResizeTime;

	f64 LastFrameTime = 0;
	std::atomic<f32> TotalRenderTime = { 0 };
	u32 GameFrameCount = 0;

	f64 LastTeleportTime = -1;
	f32 UnderWaterDepth = 0;

	f32 RenderAlpha = 0;
	KTimePoint LastGameUpdateTime;
	f64 GameTimeDilation = 1;

	bool bLoadingMap = false;
	bool bIsMainMenu = false;
	bool bResettingMap = false;
	KString LoadingMapName;

	bool bPendingScreenshot = false;

public:

	std::atomic<bool> bPendingRescale = { false };


	KRenderBuffers RenderBufferPointers;
	//KRenderBufferInfo::BufferMap RenderBufferPointers;

	KFrameStats FrameStats;

	TVector<KDamageNumber> LiveDamageNumbers;

private:

	glm::mat4 ActiveMVP;

private:

	// called to start the render thread
	void StartRenderThread();

	// finishes initialization in the render thread
	void FinishInit();

	// runs for the life of the instance
	void DrawLoop();

	void CreateShaders() {}

public:	

	void InitializeInterface();
	virtual void InitializeBackend() = 0;
	virtual void FinalizeBackend() = 0;

	KRenderInterface(bool new_thread = true);
	virtual ~KRenderInterface();

public:
	
	// declared in d3d11_create.cpp
	static UPtr<KRenderInterface> CreateD3D11(bool new_thread = true);

	// declared in opengl_create.cpp
	static UPtr<KRenderInterface> CreateOpenGL(bool new_thread = true);

public:

	/* virtual interface */

	// creates a window according to user config settings
	// cast EWindowState to u8 so we dont have to include the game window header
	virtual UPtr<class KGameWindow> CreateGameWindow(u32 resX, u32 resY, u8 state) = 0;
	
	// create the rasterizer state, initialized to cull none and fill
	virtual UPtr<class KRasterizerState> CreateRasterizerState() = 0;

	// create a depth buffer and enable it
	virtual UPtr<class KDepthBuffer> CreateDepthBuffer(EDepthUsage usage = EDepthUsage::SceneDepth, u32 w = 0, u32 h = 0) = 0;

	// clears the back buffer to a rgba color
	virtual void ClearBackBuffer(f32 r = 0, f32 g = 0, f32 b = 0, f32 a = 1) = 0;

	// binds the depth buffer as a shader resource view
	virtual void BindDepthBufferTexture(u8 slot = 0) = 0;

	// sets the current viewport to draw to
	virtual void SetViewport(u16 width, u16 height, u16 x = 0, u16 y = 0) = 0;

	// swap and present the back buffer
	virtual void Present() = 0;

	// binds a vertex buffer to the pipeline
	virtual void BindVertexBuffer(class KVertexBuffer* buffer, class KGpuBuffer* instanceBuffer = nullptr) = 0;

	// creates a blank shader program 
	virtual UPtr<class KShaderProgram> CreateShaderProgram() = 0;
	
	// binds a constant buffer to the specified slot
	virtual void BindBuffer(class KGpuBuffer* buffer) = 0;

	// binds the back buffer as the render target
	virtual void BindBackBuffer(bool depth = true) = 0;

	virtual void DrawIndexed(u32 indexCount, u32 indexStart) = 0;
	virtual void DrawInstanced(u32 vertCountPer, u32 instanceCount, u32 startIndex = 0, u32 startInstance = 0) = 0;
	virtual void Draw(u32 vertexCount, u32 startVertex) = 0;
	virtual void Dispatch(u32 countX, u32 countY, u32 countZ) = 0;

	// creates a new texture2d object
	// called by LoadTexture2D, do not call this directly
	virtual UPtr<class KTexture2D> CreateTexture2D() = 0;

	virtual void BindTexture2D(KTexture2D* tex, u32 slot, EShaderStage stage = EShaderStage::Pixel) = 0;

	virtual UPtr<class KGpuBuffer> CreateConstantBufferStatic(
		u32 typeSize, i32 slot, EShaderStage stage) = 0;

	virtual UPtr<class KGpuBuffer> CreateConstantBufferDynamic(
		u32 typeSize, i32 slot, EShaderStage stage) = 0;

	virtual UPtr<class KGpuBuffer> CreateStructuredBufferStatic(
		u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) = 0;

	virtual UPtr<class KGpuBuffer> CreateStructuredBufferDynamic(
		u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) = 0;

	virtual UPtr<class KGpuBuffer> CreateInstanceBufferStatic(
		u32 typeSize, u32 numElements) = 0;

	virtual UPtr<class KGpuBuffer> CreateInstanceBufferDynamic(
		u32 typeSize, u32 numElements) = 0;

	virtual UPtr<class KVertexBuffer> CreateVertexBufferDynamic(
		const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) = 0;

	virtual UPtr<class KVertexBuffer> CreateVertexBufferStatic(
		const void* vertices, u32 vert_size, u32 vert_count, const TVector<u32>& indices = {}) = 0;

	virtual UPtr<class KRenderTarget> CreateRenderTarget(
		u32 w, u32 h, const struct FRenderTargetCreationFlags& flags) = 0;

	virtual void BindRenderTarget(class KRenderTarget* target, class KDepthBuffer* depth = nullptr, class KUnorderedAccessView** uav = nullptr, u8 vCount = 0) = 0;
	virtual void BindRenderTargets(class KRenderTarget** targets, u8 rtCount, class KDepthBuffer* depth = nullptr, class KUnorderedAccessView** uav = nullptr, u8 vCount = 0) = 0;

	virtual void SetUAVForCS(class KUnorderedAccessView* view) = 0;
	virtual void SetUAVsForCS(class KUnorderedAccessView** views, u8 vCount) = 0;

	virtual void SetNullRenderTarget() = 0;

	virtual void SetTopology_LineStrip() = 0;
	virtual void SetTopology_LineList() = 0;
	virtual void SetTopology_TriangleList() = 0;

	virtual UPtr<class KBlendState> CreateBlendState(const struct KBlendData& data) = 0;
	virtual UPtr<class KBlendState> CreateBlendState(struct KBlendData* data, u32 blendCount) = 0;
	virtual void BindBlendState(class KBlendState* state) = 0;
	virtual void ClearBlendState() = 0;

	virtual void HandleResize() = 0;
	
	virtual void ScreenshotBackBuffer() = 0;

	virtual void UpdateRenderScaleSampler(i32 forceValue =-1) = 0;

	// HUD functions
	// backends are responsible for managing fonts and other implementation specifics
	// d3d11 uses direct2d
	// opengl might use SDL if that ever gets made
	virtual void HUD_Begin() = 0;
	virtual void HUD_SetDrawColor(const class FColor32& color) = 0;		
	virtual void HUD_SetDrawColor(class FColor8 color) = 0;		
	virtual void HUD_DrawLine(KHudPointF a, KHudPointF b, f32 thickness = 1.f) = 0;
	virtual void HUD_DrawLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 thickness = 1.f) = 0;
	virtual void HUD_DrawRect(const KHudRectF& r, f32 thickness = 1.f) = 0;
	virtual void HUD_FillRect(const KHudRectF& r) = 0;
	virtual void HUD_FillEllipse(f32 x, f32 y, f32 radX, f32 radY) = 0;
	virtual void HUD_End() = 0;
	virtual void* HUD_GetFont(EFontUsage usage) = 0;
	virtual void* HUD_LoadFont(const class KString& name, f32 size, EFontUsage usage, bool bold = false, bool italic = false) = 0;
	virtual UPtr<class KHudBitmap> HUD_LoadImage(const class KString& name) = 0;
	virtual u32 HUD_CreateTextLayout(const KString& text, EFontUsage font, f32 maxW = 0, f32 maxH = 0, u32 existingHandle = 0) = 0;
	virtual void HUD_DrawTextLayout(u32 handle, KHudPointF origin) = 0;
	virtual void HUD_SetTextLayoutFontSize(u32 handle, f32 size, i32 startPosition = 0, i32 length = -1) = 0;
	virtual void HUD_DeleteTextLayout(u32 handle) = 0;
	virtual f32 HUD_GetFontSize(u32 handle) = 0;
	virtual f32 HUD_GetFontSize(EFontUsage font) = 0;
	virtual EFontUsage HUD_GetFontFromHandle(u32 handle) = 0;
	virtual u32 HUD_GetTextLineCount(u32 handle) = 0;
	virtual f32 HUD_GetTextHeight(u32 handle) = 0;
	virtual f32 HUD_GetTextWidth(u32 handle) = 0; // include trailing whitespace
	virtual void HUD_DrawBitmap(class KHudBitmap* bmp, f32 x, f32 y, f32 w = 0, f32 h = 0, f32 alpha = 1) = 0;
	virtual void HUD_DrawCircleProgressBar(f32 centerX, f32 centerY, f32 radius, f32 progress) = 0;
	virtual void HUD_SetRadialBrushForPowerup(u8 index, f32 alpha = 1) = 0;
	virtual void HUD_SetBitmapBrush(class KHudBitmap* bmp, f32 xpos, f32 ypos, f32 xscale, f32 yscale, f32 alpha = 1, bool repeat = false) = 0;
	virtual void HUD_SetLinearGradientBrush(u8 index) = 0;
	virtual void HUD_LoadFontsFromWad() = 0;

	// control depth buffer
	void EnableDepthBuffer();
	void DisableDepthBuffer();
	void SetDepthBufferReadOnly(bool read_only);

	// clears the scene depth buffer
	virtual void ClearDepthBuffer();

	// updates an existing constant buffer with new data
	void UpdateBuffer(class KGpuBuffer* buffer, const void* data, u32 elemCount = 0);

	void DrawDebugLines();

	void DrawOpaqueBuffers(bool viewWeapon);
	void DrawTransparentBuffers(bool viewWeapon);

	void BlurRenderTarget(KRenderTarget* target, bool apply = true);
	void CreateDownsampleDepthBuffer();


protected:

	// creates or recreates the PostTarget render target
	void CreateRenderTargets();

	// loads all shaders
	virtual void LoadShaders();

private:

	void CreateBlendStates();

	// calls create window and assigns it to Window
	void CreateAndSetGameWindow(u32 resX, u32 resY, u8 state);

	void NotifyWidgetsResize();

	// initializes various constant buffers across different systems
	void InitConstantBuffers();

	void CreateMaterials();

	void SetWorldShader(u8 index);

	public:
	void OnNewMapLoaded();
	private:

	void MergeTargetDomains(KRenderTarget* target = nullptr);


	void LightOpaque();

	void RenderFinalTargetToBackBuffer();
	void ProcessPendingScreenshot();

	void UpdatePerFrameData();
	
	void TakeScreenshot();

	void UpdateFog(const TVector<KFogBuffer>& fog);
	void ClearFog();

protected:

	// loads fonts sized for the current window
	void LoadFonts();

public:

	void BindGBuffer();

	void AllocateScenes();

	std::function<void()> CreateBufferFunc;
	std::atomic<bool> bMemPoolsCreated = { false };
	std::atomic<bool> bPendingBufferFunc = { false };
	std::mutex BufferMutex;

	std::mutex CommandMutex;
	TVector<KPendingConsoleCommand> PendingConsoleCommands;

	class KRenderScene* GetWorldScene();
	class KRenderScene* GetSkyboxScene();
	bool HasRenderThread() const { return bRenderThread; }
	bool IsInitialized() const { return bInitialized; }
	class KGameWindow* GetGameWindow() const { return Window.get(); }
	class KRasterizerState* GetRasterizer() { return Rasterizer.get(); }

	bool IsBuildingRadiosity() { return Radiosity.bBuildingRadiosity; }

	f32 GetTotalRenderTime() const { return TotalRenderTime; }
	u32 GetGameFrameCount() const { return GameFrameCount; }

	void EnableTransparency();
	void DisableTransparency();

	void EnableWaterBlend();
	void DisableWaterBlend();

	void ResizeResolution(f32 x, f32 y);

	class KDrawConfig* GetDrawConfig() const;

	//class KTexture2D* GetTexture2D(const KString& name);

	f64 GetLastFrameTime() const { return LastFrameTime; }

	f32 GetGameTimeDilation() const { return GameTimeDilation; }

	f32 GetRenderAlpha() const { return RenderAlpha; }

	KRenderCamera& GetCamera() { return Camera; }

	// binds all shaders contained within a program
	void BindShaderProgram(class KShaderProgram* program);

	UPtr<KTexture2D> CreateTexture2DFromSurface(UPtr<class KSurface2D> surface);
	UPtr<KTexture2D> CreateTexture2DFromFreetypeGlyph(struct FT_GlyphSlotRec_* glyph);
	UPtr<KTexture2D> LoadTexture2D(const class KString& name);

	// create an index buffer (null vertex buffer) for point sprites
	UPtr<class KVertexBuffer> CreateParticleIndexBuffer(u32 count);

	// where it all happens
	// defined in render/frame.cpp
	void PrepFrame();
	void DrawScene();
	void DrawHud();

	// binds the current world shader and updates cull and fill modes
	void BindWorldShader();

	// binds the model view projection matrix to cb slot 0
	void SetModelViewProjection(const glm::mat4& mvp);

	// binds the model transform matrix to cb slot 1
	void SetModelTransform(const glm::mat4 model);

	// sets model and mvp buffers, uses camera viewprojection matrix
	void SetMvpFromModel(const glm::mat4 model);

	// processes updates to settings that were deferred until the next render bridge read
	void ProcessCommand(u64 command);

	// called from main thread to set up a radiosity test
	DVec3 CalculateRadiosityAtPoint(const DVec3& point, const DVec3& norm, class KBrushFace* face);

	// runs in render thread
	//void CalculatePendingRadiosity(); 

	bool IsPendingRadiosityTest() { return Radiosity.bPendingTest; }
	DVec3 GetRadiosityColorResult() { return Radiosity.ColorResult; }

	void SetBuildingRadiosity(bool build) { Radiosity.bBuildingRadiosity = build; }

	f32 GetTimeDilation() const { return GameTimeDilation; }

	bool IsLoadingMap() const { return bLoadingMap; }
	bool IsResettingMap() const { return bResettingMap; }
	const KString& GetLoadingMapName() const { return LoadingMapName; }

	glm::vec2 ProjectWorldToScreen(const glm::vec3& worldPoint);

	bool IsShowingScoreboard() const;

private:

	void AddPrecipitation(const class KPrecipitationVolume& p);

	// called from game instance before resetting the interface pointer
	// cannot be called from destructor because it nullifies GetRenderInterface()
	//		as soon as interface pointer is reset, and we need to wait for render thread to join
	void CallInterfaceDestruction();
};

/*********** globals **********/

// easy get function to avoid getting through game instance
KRenderInterface* GetRenderInterface();

f32 GetRenderResX();
f32 GetRenderResY();
f32 GetViewportX();
f32 GetViewportY();
f32 GetScaledX();
f32 GetScaledY();
f32 GetRenderScale();
f32 GetViewportPixelCount();

#endif
