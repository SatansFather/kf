#pragma once

#if !_SERVER && _WIN32


#include "engine/render/interface/render_interface.h"

class KString;

// includes for template functions that need to be defined in header (damn that sucks)
#include "buffers/d3d11_vertex_buffer.h"

class KRenderInterface_D3D11 : public KRenderInterface
{
public:
	UPtr<class KD2DInterface> D2D;
private:

	ComPtr<ID3D11Device> Device;
	ComPtr<ID3D11DeviceContext> ImmediateContext;
	ComPtr<IDXGISwapChain> SwapChain;
	UPtr<class KRenderTarget_D3D11> BackBuffer;


	void CreateDevice();
	void CreateSwapChain();
	void CreateBackBuffer();

	// allows us to get a raw pointer to the depth buffer without casting 
	ID3D11DepthStencilView* SceneDepth = nullptr;

public:	

	KRenderInterface_D3D11(bool new_thread);
	~KRenderInterface_D3D11();

	void InitializeBackend() override;
	void FinalizeBackend() override;
	UPtr<class KGameWindow> CreateGameWindow(u32 resX, u32 resY, u8 state) override;
	UPtr<class KRasterizerState> CreateRasterizerState() override;
	UPtr<class KDepthBuffer> CreateDepthBuffer(EDepthUsage usage = EDepthUsage::SceneDepth, u32 w = 0, u32 h = 0) override;
	void ClearBackBuffer(f32 r = 0, f32 g = 0, f32 b = 0, f32 a = 1) override;
	void BindDepthBufferTexture(u8 slot = 0) override;
	void SetViewport(u16 width, u16 height, u16 x = 0, u16 y = 0) override;
	void Present() override;
	void BindVertexBuffer(class KVertexBuffer* buffer, class KGpuBuffer* instanceBuffer = nullptr) override;
	void BindBackBuffer(bool depth = true) override;
	void DrawIndexed(u32 index_count, u32 index_start) override;
	void DrawInstanced(u32 vertCountPer, u32 instanceCount, u32 startIndex = 0, u32 startInstance = 0) override;
	void Draw(u32 vertex_count, u32 start_vertex) override;
	void Dispatch(u32 countX, u32 countY, u32 countZ) override;
	UPtr<class KShaderProgram> CreateShaderProgram() override;
	void BindBuffer(class KGpuBuffer* buffer) override;
	UPtr<class KTexture2D> CreateTexture2D() override;
	void BindTexture2D(KTexture2D* tex, u32 slot, EShaderStage stage = EShaderStage::Pixel) override;
	UPtr<class KGpuBuffer> CreateConstantBufferStatic(
		u32 typeSize, i32 slot, EShaderStage stage) override;

	UPtr<class KGpuBuffer> CreateConstantBufferDynamic(
		u32 typeSize, i32 slot, EShaderStage stage) override;

	UPtr<class KGpuBuffer> CreateStructuredBufferStatic(
		u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) override;

	UPtr<class KGpuBuffer> CreateStructuredBufferDynamic(
		u32 typeSize, i32 slot, EShaderStage stage, u32 numElements) override;

	UPtr<class KGpuBuffer> CreateInstanceBufferStatic(
		u32 typeSize, u32 numElements) override;

	UPtr<class KGpuBuffer> CreateInstanceBufferDynamic(
		u32 typeSize, u32 numElements) override;

	UPtr<class KVertexBuffer> CreateVertexBufferDynamic(
		const void* vertices, u32 vertSize, u32 vertCount, const TVector<u32>& indices = {}) override;

	UPtr<class KVertexBuffer> CreateVertexBufferStatic(
		const void* vertices, u32 vertSize, u32 vertCount, const TVector<u32>& indices = {}) override;

	UPtr<class KRenderTarget> CreateRenderTarget(
		u32 w, u32 h, const struct FRenderTargetCreationFlags& flags) override;

	void BindRenderTarget(class KRenderTarget* target, class KDepthBuffer* depth = nullptr, class KUnorderedAccessView** uav = nullptr, u8 vCount = 0) override;
	void BindRenderTargets(class KRenderTarget** targets, u8 count, class KDepthBuffer* depth = nullptr, class KUnorderedAccessView** uav = nullptr, u8 vCount = 0) override;
	void SetUAVForCS(class KUnorderedAccessView* view) override;
	void SetUAVsForCS(class KUnorderedAccessView** views, u8 vCount) override;

	void SetNullRenderTarget() override;

	void SetTopology_LineStrip() override;
	void SetTopology_LineList() override;
	void SetTopology_TriangleList() override;

	UPtr<class KBlendState> CreateBlendState(const struct KBlendData& data) override;
	UPtr<class KBlendState> CreateBlendState(struct KBlendData* data, u32 blendCount) override;

	void BindBlendState(class KBlendState* state) override;
	void ClearBlendState() override;

	void HandleResize() override;

	void ScreenshotBackBuffer() override;

	void UpdateRenderScaleSampler(i32 forceValue =-1) override;

	void HUD_Begin() override;
	void HUD_SetDrawColor(const class FColor32& color) override;
	void HUD_SetDrawColor(class FColor8 color) override;
	void HUD_DrawLine(KHudPointF a, KHudPointF b, f32 thickness = 1.f) override;
	void HUD_DrawLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 thickness = 1.f) override;
	void HUD_DrawRect(const KHudRectF& r, f32 thickness = 1.f) override;
	void HUD_FillRect(const KHudRectF& r) override;
	void HUD_FillEllipse(f32 x, f32 y, f32 radX, f32 radY) override;
	void HUD_End() override;
	void* HUD_GetFont(EFontUsage usage) override;
	void* HUD_LoadFont(const class KString& name, f32 size, EFontUsage usage, bool bold = false, bool italic = false) override;
	UPtr<class KHudBitmap> HUD_LoadImage(const class KString& name) override;
	u32 HUD_CreateTextLayout(const KString& text, EFontUsage font, f32 maxW = 0, f32 maxH = 0, u32 existingHandle = 0) override;
	void HUD_DrawTextLayout(u32 handle, KHudPointF origin) override;
	void HUD_SetTextLayoutFontSize(u32 handle, f32 size, i32 startPosition = 0, i32 length = -1) override;
	void HUD_DeleteTextLayout(u32 handle) override;
	f32 HUD_GetFontSize(u32 handle) override;
	f32 HUD_GetFontSize(EFontUsage font) override;
	EFontUsage HUD_GetFontFromHandle(u32 handle) override;
	u32 HUD_GetTextLineCount(u32 handle) override;
	f32 HUD_GetTextHeight(u32 handle) override;
	f32 HUD_GetTextWidth(u32 handle) override;
	void HUD_DrawBitmap(class KHudBitmap* bmp, f32 x, f32 y, f32 w = 0, f32 h = 0, f32 alpha = 1) override;
	void HUD_DrawCircleProgressBar(f32 centerX, f32 centerY, f32 radius, f32 progress) override;
	void HUD_SetRadialBrushForPowerup(u8 index, f32 alpha = 1) override;
	void HUD_SetBitmapBrush(class KHudBitmap* bmp, f32 xpos, f32 ypos, f32 xscale, f32 yscale, f32 alpha = 1, bool repeat = false) override;
	void HUD_SetLinearGradientBrush(u8 index) override;
	void HUD_LoadFontsFromWad() override;

	void LoadShaders() override;

public:

	TVector<struct IDXGIAdapter*> EnumerateAdapters();

	void CreateBitmapScaler(ComPtr<struct IWICBitmapScaler>& scaler);
	void CreateBitmapFromWicBitmap(struct IWICBitmapScaler* scaler, class KHudBitmap_D3D11* bmp);

	ID3D11DeviceContext* GetImmediateContext() const { return ImmediateContext.Get(); }
	ID3D11Device* GetDevice() const { return Device.Get(); }
};

/*********** globals ***********/

KRenderInterface_D3D11* GetD3D11Interface();
ID3D11DeviceContext* GetD3D11ImmediateContext();

#endif