#if !_SERVER && _WIN32

#include "d3d11_interface.h"
#include "d3d11_window.h"
#include "engine/utility/k_assert.h"
#include "d3d11_rasterizer.h"
#include "d3d11_render_target.h"
#include "engine/utility/kstring.h"
#include "d3d11_depth_buffer.h"
#include "engine/global/paths.h"
#include "d3d11_shader_program.h"
#include "buffers/d3d11_gpu_buffer.h"
#include "d3d11_texture2d.h"
#include "dxgi.h"
#include "d2d/d2d_interface.h"
#include "../../bufferslots.h"
#include "gdi_init.h"
#include "d3d11_blend_state.h"
#include "d3d11_bitmap.h"
#include "../../../game/config.h"

// DELETE
#include "../../../math/line_segment.h"
#include "../../../utility/random.h"
#include "../../vertex_types.h"
#include "../../../game_instance.h"

#include "d2d/suffering/ResourceFontCollectionLoader.h"
#include "d2d/suffering/ResourceFontFileLoader.h"

KRenderInterface_D3D11* GetD3D11Interface()
{
	return dynamic_cast<KRenderInterface_D3D11*>(GetRenderInterface());
}

ID3D11DeviceContext* GetD3D11ImmediateContext()
{
	KRenderInterface_D3D11* iface = GetD3D11Interface();
	K_ASSERT(iface, "No D3D11 Interface available");
	return iface->GetImmediateContext();
}

KRenderInterface_D3D11::KRenderInterface_D3D11(bool new_thread) : KRenderInterface(new_thread)
{

}

KRenderInterface_D3D11::~KRenderInterface_D3D11() 
{
	D2D->WriteFactory->UnregisterFontCollectionLoader(ResourceFontCollectionLoader::GetLoader());
	D2D->WriteFactory->UnregisterFontFileLoader(ResourceFontFileLoader::GetLoader());
}

void KRenderInterface_D3D11::CreateDevice()
{
#if 0
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#else
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#endif
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&Device,
		NULL,
		&ImmediateContext);
		
#else

	TVector<IDXGIAdapter*> adapters = EnumerateAdapters();
	for (IDXGIAdapter* ad : adapters)
	{
		DXGI_ADAPTER_DESC adesc;
		ad->GetDesc(&adesc);
		KString adapterdesc(adesc.Description);
		LOG("GPU detected: " + adapterdesc);
		break;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	DXGI_MODE_DESC buffer_desc;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	ZeroMemory(&buffer_desc, sizeof(DXGI_MODE_DESC));

	KGameWindow_D3D11* window = dynamic_cast<KGameWindow_D3D11*>(GetGameWindow());

	buffer_desc.Width = window->GetWidth();
	buffer_desc.Height = window->GetHeight();
	buffer_desc.RefreshRate.Numerator = 0;
	buffer_desc.RefreshRate.Denominator = 0;
	buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.BufferDesc = buffer_desc;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = window->hWnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		//nullptr,
		//D3D_DRIVER_TYPE_HARDWARE,
		adapters[0],
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#else
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
#endif
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&SwapChain,
		&Device,
		NULL,
		&ImmediateContext);

#endif

	ComPtr<IDXGIDevice1> dxgiDevice;
	Device.As(&dxgiDevice);

	dxgiDevice->SetMaximumFrameLatency(1);

	// debug check prevents the function call from going into the assert directly
	K_ASSERT_HR(hr, "could not create d3d11 device");
}

void KRenderInterface_D3D11::CreateSwapChain()
{	
	return;

	DXGI_SWAP_CHAIN_DESC scd;
	DXGI_MODE_DESC buffer_desc;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	ZeroMemory(&buffer_desc, sizeof(DXGI_MODE_DESC));

	KGameWindow_D3D11* window = dynamic_cast<KGameWindow_D3D11*>(GetGameWindow());

	buffer_desc.Width = window->GetWidth();
	buffer_desc.Height = window->GetHeight();
	buffer_desc.RefreshRate.Numerator = 0;
	buffer_desc.RefreshRate.Denominator = 0;
	buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.BufferDesc = buffer_desc;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = window->hWnd;
	scd.Windowed = FALSE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	// bunch of crap:

	ComPtr<IDXGIDevice1> dxgi_device;
	Device.As(&dxgi_device);

	ComPtr<IDXGIAdapter> dxgi_adapter;
	dxgi_device->GetAdapter(&dxgi_adapter);

	ComPtr<IDXGIFactory> dxgi_factory;
	dxgi_adapter->GetParent(__uuidof(IDXGIFactory), &dxgi_factory);

	K_ASSERT_HR(dxgi_factory->CreateSwapChain(Device.Get(), &scd, &SwapChain),
		"could not create swap chain");
}

void KRenderInterface_D3D11::CreateBackBuffer()
{
	// back buffer is a render target but has unique properties
	// dont initialize it like a normal render target

	FRenderTargetCreationFlags flags;
	BackBuffer = std::make_unique<KRenderTarget_D3D11>();

	// get the address of the swap chain's back buffer 
	ID3D11Texture2D* backBuffer = nullptr;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	K_ASSERT(backBuffer, "cannot fill back buffer texture");

	// assign the render target view of our new back buffer object to the swap chain's buffer
	// RenderTarget member is private but this is a friend class because we need the pointer directly
	Device->CreateRenderTargetView(backBuffer, NULL, &BackBuffer->RenderTarget);

	D3D11_TEXTURE2D_DESC td;
	backBuffer->GetDesc(&td);

	BackBuffer->Width = td.Width;
	BackBuffer->Height = td.Height;

	backBuffer->Release();
	
	D2D->CreateD2DSurfaceForD3D11(SwapChain.Get());
}

void KRenderInterface_D3D11::InitializeBackend()
{
	K_ASSERT(!bInitialized, "Tried initializing renderer a second time");
	
	CallInitGDI();

	CreateDevice();

	D2D = std::make_unique<KD2DInterface>();
	D2D->InitD2D();

	CreateSwapChain();
	CreateBackBuffer();

	{
		ComPtr<ID3D11SamplerState> UnfilteredSamplerState;
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		Device->CreateSamplerState(&sd, &UnfilteredSamplerState);
		ImmediateContext->PSSetSamplers(SMP_UNFILTERED, 1, UnfilteredSamplerState.GetAddressOf());
		ImmediateContext->CSSetSamplers(SMP_UNFILTERED, 1, UnfilteredSamplerState.GetAddressOf());
	}

	{
		ComPtr<ID3D11SamplerState> LightSamplerState;
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		sd.BorderColor[0] = 0.f;
		sd.BorderColor[1] = 0.f;
		sd.BorderColor[2] = 0.f;
		sd.BorderColor[3] = 0.f;

		Device->CreateSamplerState(&sd, &LightSamplerState);
		ImmediateContext->PSSetSamplers(SMP_SHADOWMAP, 1, LightSamplerState.GetAddressOf());
		ImmediateContext->CSSetSamplers(SMP_SHADOWMAP, 1, LightSamplerState.GetAddressOf());
	}

	{
		ComPtr<ID3D11SamplerState> FilteredSamplerState;
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		Device->CreateSamplerState(&sd, &FilteredSamplerState);
		ImmediateContext->PSSetSamplers(SMP_FILTERED, 1, FilteredSamplerState.GetAddressOf());
		ImmediateContext->CSSetSamplers(SMP_FILTERED, 1, FilteredSamplerState.GetAddressOf());
	}

	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	LOG("D3D11 Initialized");
}

void KRenderInterface_D3D11::FinalizeBackend()
{
	SceneDepth = dynamic_cast<KDepthBuffer_D3D11*>(SceneDepthBuffer.get())->GetViewPointer();
}

UPtr<KGameWindow> KRenderInterface_D3D11::CreateGameWindow(u32 resX, u32 resY, u8 state)
{	
	return KGameWindow_D3D11::CreateGameWindowD3D(resX, resY, state);
}

UPtr<class KRasterizerState> KRenderInterface_D3D11::CreateRasterizerState()
{
	return std::make_unique<KRasterizerState_D3D11>();
}

UPtr<class KDepthBuffer> KRenderInterface_D3D11::CreateDepthBuffer(EDepthUsage usage, u32 w, u32 h)
{
	if (w == 0) w = GetRenderResX();
	if (h == 0) h = GetRenderResY();
	return std::make_unique<KDepthBuffer_D3D11>(usage, w, h);
}

void KRenderInterface_D3D11::ClearBackBuffer(f32 r /*= 0*/, f32 g /*= 0*/, f32 b /*= 0*/, f32 a /*= 1*/)
{
	BackBuffer->Clear(r, g, b, a);
}

void KRenderInterface_D3D11::BindDepthBufferTexture(u8 slot /*= 0*/)
{	
	SceneDepthBuffer->BindTexture2D(slot);
}

void KRenderInterface_D3D11::SetViewport(u16 width, u16 height, u16 x /*= 0*/, u16 y /*= 0*/)
{
	D3D11_VIEWPORT view;
	
	view.MinDepth = 0;
	view.MaxDepth = 1;
	view.TopLeftX = x;
	view.TopLeftY = y;
	view.Width = width;
	view.Height = height;

	ImmediateContext->RSSetViewports(1, &view);	
}

void KRenderInterface_D3D11::Present()
{
#if _COMPILER
	SwapChain->Present(1, 0);
#else
	SwapChain->Present(0, 0);
#endif
}

void KRenderInterface_D3D11::BindVertexBuffer(class KVertexBuffer* buffer, class KGpuBuffer* instanceBuffer)
{
	if (buffer)
	{
		if (buffer->GetVertexCount() == 0)
		{
			ImmediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		}
		else
		{
			if (instanceBuffer)
			{
				u32 offset[] = { 0, 0 };
				u32 stride[] = { buffer->GetVertexSize(), instanceBuffer->GetTypeSize() };
				ID3D11Buffer* buffs[] = 
				{ 
					dynamic_cast<KVertexBuffer_D3D11*>(buffer)->VertexBuffer.Get(),
					dynamic_cast<KGpuBuffer_D3D11*>(instanceBuffer)->Buffer.Get() 
				};
				ImmediateContext->IASetVertexBuffers(0, 2,
					buffs, stride, offset);
			}
			else
			{
				u32 offset = 0;
				u32 stride = buffer->GetVertexSize();
				ImmediateContext->IASetVertexBuffers(0, 1, 
					(ID3D11Buffer* const*)buffer->GetVertexBufferAddress(), &stride, &offset);
			}
		}

		if (buffer->UsesIndexBuffer())
		{
			ImmediateContext->IASetIndexBuffer(
				(ID3D11Buffer*)buffer->GetIndexBufferPointer(), DXGI_FORMAT_R32_UINT, 0);
		}
	}
	else
	{
		// unbind buffer
		ImmediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		ImmediateContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		//ID3D11Buffer* null[1] = { nullptr };
		//u32 offset = 0, stride = 0;
		//ImmediateContext->IASetVertexBuffers(0, 1, null, &stride, &offset);
		//ImmediateContext->IASetIndexBuffer(*null, DXGI_FORMAT_R32_UINT, 0);
	}
}

void KRenderInterface_D3D11::BindBackBuffer(bool depth /*= true*/)
{
	ImmediateContext->OMSetRenderTargets
	(
		1, 
		BackBuffer->GetRenderTargetViewAddress(), 
		depth ? SceneDepth : nullptr
	);
}

void KRenderInterface_D3D11::DrawIndexed(u32 indexCount, u32 indexStart)
{
	if (indexCount == 0) return;

	ImmediateContext->DrawIndexed(indexCount, indexStart, 0);
	FrameStats.DrawCalls++;
	FrameStats.Triangles += indexCount / 3;
}

void KRenderInterface_D3D11::DrawInstanced(u32 vertCountPer, u32 instanceCount, u32 startIndex /*= 0*/, u32 startInstance /*= 0*/)
{
	if (instanceCount == 0 || vertCountPer == 0) return;
	
	ImmediateContext->DrawInstanced(vertCountPer, instanceCount, startIndex, startInstance);
	FrameStats.DrawCalls++;
	FrameStats.Triangles += vertCountPer / 3 * instanceCount;
}

void KRenderInterface_D3D11::Draw(u32 vertexCount, u32 startVertex)
{
	ImmediateContext->Draw(vertexCount, startVertex);
	FrameStats.DrawCalls++;
	FrameStats.Triangles += vertexCount / 3;
}

void KRenderInterface_D3D11::Dispatch(u32 countX, u32 countY, u32 countZ)
{
	ImmediateContext->Dispatch(countX, countY, countZ);
}

UPtr<class KShaderProgram> KRenderInterface_D3D11::CreateShaderProgram()
{
	return std::make_unique<KShaderProgram_D3D11>();
}

void KRenderInterface_D3D11::BindBuffer(class KGpuBuffer* buffer)
{
	auto buff = dynamic_cast<KGpuBuffer_D3D11*>(buffer);
	K_ASSERT(buff, "d3d11 interface cannot bind non-d3d11 constant buffer");

	switch (buff->GetShaderStage())
	{
		case EShaderStage::Pipeline:
		{
			if (buff->SRV.Get())
			{
				ImmediateContext->VSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
				ImmediateContext->PSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
			}
			else
			{
				ImmediateContext->VSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
				ImmediateContext->PSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
			}
			break;
		}
		case EShaderStage::All:
		{
			if (buff->SRV.Get())
			{
				ImmediateContext->VSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
				ImmediateContext->PSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
				ImmediateContext->CSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
			}
			else
			{
				ImmediateContext->VSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
				ImmediateContext->PSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
				ImmediateContext->CSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
			}
			break;
		}
		case EShaderStage::Vertex:
		{
			if (buff->SRV.Get()) ImmediateContext->VSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
			else ImmediateContext->VSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
			break;
		}
		case EShaderStage::Pixel:
		{
			if (buff->SRV.Get()) ImmediateContext->PSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
			else ImmediateContext->PSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
			break;
		}
		case EShaderStage::Compute:
		{
			if (buff->SRV.Get()) ImmediateContext->CSSetShaderResources(buff->Slot, 1, buff->SRV.GetAddressOf());
			else ImmediateContext->CSSetConstantBuffers(buff->Slot, 1, buff->Buffer.GetAddressOf());
			break;
		}
	}
}

UPtr<class KTexture2D> KRenderInterface_D3D11::CreateTexture2D()
{
	return std::make_unique<KTexture2D_D3D11>();
}

void KRenderInterface_D3D11::BindTexture2D(KTexture2D* tex, u32 slot, EShaderStage stage)
{
	KTexture2D_D3D11* t = dynamic_cast<KTexture2D_D3D11*>(tex);
	ID3D11ShaderResourceView* view = t ? t->GetResource() : nullptr;

	switch(stage)
	{
		case EShaderStage::Pipeline:
			ImmediateContext->VSSetShaderResources(slot, 1, &view);
			ImmediateContext->PSSetShaderResources(slot, 1, &view);
			break;
		case EShaderStage::All:
			ImmediateContext->VSSetShaderResources(slot, 1, &view);
			ImmediateContext->PSSetShaderResources(slot, 1, &view);
			ImmediateContext->CSSetShaderResources(slot, 1, &view);
			break;
		case EShaderStage::Vertex:
			ImmediateContext->VSSetShaderResources(slot, 1, &view);
			break;
		case EShaderStage::Pixel:
			ImmediateContext->PSSetShaderResources(slot, 1, &view);
			break;
		case EShaderStage::Compute:
			ImmediateContext->CSSetShaderResources(slot, 1, &view);
			break;
	}
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateConstantBufferStatic(u32 typeSize, i32 slot, EShaderStage stage)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateConstantStatic(typeSize, slot, stage);
	return buffer;
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateConstantBufferDynamic(u32 typeSize, i32 slot, EShaderStage stage)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateConstantDynamic(typeSize, slot, stage);
	return buffer;
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateStructuredBufferStatic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateStructuredStatic(typeSize, slot, stage, numElements);
	return buffer;
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateStructuredBufferDynamic(u32 typeSize, i32 slot, EShaderStage stage, u32 numElements)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateStructuredDynamic(typeSize, slot, stage, numElements);
	return buffer;
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateInstanceBufferStatic(u32 typeSize, u32 numElements)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateInstancedStatic(typeSize, numElements);
	return buffer;
}

UPtr<class KGpuBuffer> KRenderInterface_D3D11::CreateInstanceBufferDynamic(u32 typeSize, u32 numElements)
{
	UPtr<KGpuBuffer_D3D11> buffer = std::make_unique<KGpuBuffer_D3D11>();
	buffer->CreateInstancedDynamic(typeSize, numElements);
	return buffer;
}

UPtr<class KVertexBuffer> KRenderInterface_D3D11::CreateVertexBufferDynamic
(const void* vertices, u32 vertSize, u32 vertCount, const TVector<u32>& indices /*= {}*/)
{
	UPtr<KVertexBuffer_D3D11> buff = std::make_unique<KVertexBuffer_D3D11>();
	buff->CreateDynamic(vertices, vertSize, vertCount, indices);
	return buff;
}

UPtr<class KVertexBuffer> KRenderInterface_D3D11::CreateVertexBufferStatic
(const void* vertices, u32 vertSize, u32 vertCount, const TVector<u32>& indices /*= {}*/)
{
	UPtr<KVertexBuffer_D3D11> buff = std::make_unique<KVertexBuffer_D3D11>();
	buff->CreateStatic(vertices, vertSize, vertCount, indices);
	return buff;
}

UPtr<class KRenderTarget> KRenderInterface_D3D11::CreateRenderTarget(u32 w, u32 h, const struct FRenderTargetCreationFlags& flags)
{
	auto target = std::make_unique<KRenderTarget_D3D11>();
	target->Create(w, h, flags);
	return target;
}

UPtr<class KBlendState> KRenderInterface_D3D11::CreateBlendState(const struct KBlendData& data)
{
	auto state = std::make_unique<KBlendState_D3D11>();
	state->Finalize(data);
	return state;
}

UPtr<class KBlendState> KRenderInterface_D3D11::CreateBlendState(struct KBlendData* data, u32 blendCount)
{
	auto state = std::make_unique<KBlendState_D3D11>();
	state->Finalize(data, blendCount);
	return state;
}

void KRenderInterface_D3D11::BindBlendState(class KBlendState* state)
{
	auto s = dynamic_cast<KBlendState_D3D11*>(state);
	f32 blend[] = { 1.f, 1.f, 1.f, 1.f };
	ImmediateContext->OMSetBlendState(s->State.Get(), blend, 0xffffffff);
}

void KRenderInterface_D3D11::ClearBlendState()
{
	ImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
}

void KRenderInterface_D3D11::BindRenderTarget(KRenderTarget* target, class KDepthBuffer* depth, class KUnorderedAccessView** uav, u8 vCount)
{
	auto tar = dynamic_cast<KRenderTarget_D3D11*>(target);
	auto d = dynamic_cast<KDepthBuffer_D3D11*>(depth);
	//K_ASSERT(tar, "tried binding non-D3D11 (or null) render target");


	if (uav)
	{
		ID3D11UnorderedAccessView* uavs[8];
		for (u8 i = 0; i < vCount; i++)
		{
			ID3D11UnorderedAccessView* v = (ID3D11UnorderedAccessView*)uav[i]->GetUAV();
			uavs[i] = v;
		}
		
		if (!tar)
		{
			// bind null render target
			ID3D11RenderTargetView* nulltar[1] = { 0 };
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, nulltar, d ? d->GetViewPointer() : nullptr, 1, vCount, uavs, nullptr);
		}
		else
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, tar->GetRenderTargetViewAddress(), d ? d->GetViewPointer() : nullptr, 1, vCount, uavs, nullptr);

		return;
	}

	if (!tar)
	{
		// bind null render target
		ID3D11RenderTargetView* nulltar[1] = { 0 };
		ImmediateContext->OMSetRenderTargets(1, nulltar, d ? d->GetViewPointer() : nullptr);
	}
	else
		ImmediateContext->OMSetRenderTargets(1, tar->GetRenderTargetViewAddress(), d ? d->GetViewPointer() : nullptr);
}

void KRenderInterface_D3D11::BindRenderTargets(KRenderTarget** targets, u8 rtCount, class KDepthBuffer* depth, class KUnorderedAccessView** uav, u8 vCount)
{
	ID3D11RenderTargetView* views[8];
	for (u8 i = 0; i < rtCount; i++)
		views[i] = targets[i] ? 
			dynamic_cast<KRenderTarget_D3D11*>(targets[i])->GetRenderTargetView() :
			nullptr;

	if (uav)
	{
		ID3D11UnorderedAccessView* uavs[8];
		for (u8 i = 0; i < vCount; i++)
		{
			ID3D11UnorderedAccessView* v = (ID3D11UnorderedAccessView*)uav[i]->GetUAV();
			uavs[i] = v;
		}

		auto d = dynamic_cast<KDepthBuffer_D3D11*>(depth);
		ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(rtCount, views, d ? d->GetViewPointer() : nullptr, rtCount, vCount, uavs, nullptr);

		return;
	}

	auto d = dynamic_cast<KDepthBuffer_D3D11*>(depth);
	ImmediateContext->OMSetRenderTargets(rtCount, views, d ? d->GetViewPointer() : nullptr);
}

void KRenderInterface_D3D11::SetUAVForCS(KUnorderedAccessView* view)
{
	ID3D11UnorderedAccessView* v = view ? (ID3D11UnorderedAccessView*)view->GetUAV() : nullptr;
	ImmediateContext->CSSetUnorderedAccessViews(0, 1, &v, nullptr);
}

void KRenderInterface_D3D11::SetUAVsForCS(KUnorderedAccessView** views, u8 vCount)
{
	ID3D11UnorderedAccessView* uavs[8];
	for (u8 i = 0; i < vCount; i++)
	{
		ID3D11UnorderedAccessView* v = (ID3D11UnorderedAccessView*)views[i]->GetUAV();
		uavs[i] = v;
	}

	ImmediateContext->CSSetUnorderedAccessViews(0, vCount, uavs, nullptr);
}

void KRenderInterface_D3D11::SetNullRenderTarget()
{
	ID3D11RenderTargetView* v = nullptr;
	ImmediateContext->OMSetRenderTargets(1, &v, nullptr);
}

void KRenderInterface_D3D11::SetTopology_LineStrip()
{
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void KRenderInterface_D3D11::SetTopology_LineList()
{
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void KRenderInterface_D3D11::SetTopology_TriangleList()
{
	ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void KRenderInterface_D3D11::HandleResize()
{
	SetNullRenderTarget();
	D2D->RenderTarget.Reset();
	BackBuffer.reset();
	
	SceneDepthBuffer.reset();

	/*ID3D11Debug* debug;
	Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);*/

	K_ASSERT_HR(SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH),
		"could not resize swap chain buffer");

	CreateBackBuffer();
	SceneDepthBuffer = CreateDepthBuffer();
	SceneDepthBuffer->SetReadOnly(false);
	SceneDepthBuffer->SetEnabled(true);

	SceneDepthBuffer_Downsampled = CreateDepthBuffer(EDepthUsage::SceneDepth, GetRenderResX() / 2, GetRenderResY() / 2);

	CreateRenderTargets();

	//GetGameWindow()->MoveToCenter();
}

void KRenderInterface_D3D11::ScreenshotBackBuffer()
{
	BackBuffer->Screenshot();
}

void KRenderInterface_D3D11::UpdateRenderScaleSampler(i32 forceValue /*=-1*/)
{
#if !_COMPILER
	u8 filter;

	if (forceValue == -1)
	{
		LockConfig();
		filter = GetDrawConfig()->bFilterRenderScale;
		UnlockConfig();
	}
	else
	{
		filter = forceValue;
	}

	if (filter)
	{
		ComPtr<ID3D11SamplerState> FilteredSamplerState;
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		Device->CreateSamplerState(&sd, &FilteredSamplerState);
		ImmediateContext->PSSetSamplers(SMP_RENDER_SCALE, 1, FilteredSamplerState.GetAddressOf());
		ImmediateContext->CSSetSamplers(SMP_RENDER_SCALE, 1, FilteredSamplerState.GetAddressOf());
	}
	else
	{
		ComPtr<ID3D11SamplerState> UnfilteredSamplerState;
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		Device->CreateSamplerState(&sd, &UnfilteredSamplerState);
		ImmediateContext->PSSetSamplers(SMP_RENDER_SCALE, 1, UnfilteredSamplerState.GetAddressOf());
		ImmediateContext->CSSetSamplers(SMP_RENDER_SCALE, 1, UnfilteredSamplerState.GetAddressOf());
	}
#endif
}

void KRenderInterface_D3D11::HUD_Begin()
{
	D2D->Begin();
}

void KRenderInterface_D3D11::HUD_SetDrawColor(const FColor32& color)
{
	D2D->SetDrawColor(color);
}

void KRenderInterface_D3D11::HUD_SetDrawColor(class FColor8 color)
{
	D2D->SetDrawColor(color);
}

void KRenderInterface_D3D11::HUD_DrawLine(KHudPointF a, KHudPointF b, f32 thickness)
{
	D2D->DrawLine(a, b, thickness);
}

void KRenderInterface_D3D11::HUD_DrawLine(f32 x1, f32 y1, f32 x2, f32 y2, f32 thickness /*= 1.f*/)
{
	HUD_DrawLine( { x1, y1 }, { x2, y2 }, thickness );
}

void KRenderInterface_D3D11::HUD_DrawRect(const KHudRectF& r, f32 thickness)
{
	D2D->DrawRect(r, thickness);
}

void KRenderInterface_D3D11::HUD_FillRect(const KHudRectF& r)
{
	D2D->FillRect(r);
}

void KRenderInterface_D3D11::HUD_FillEllipse(f32 x, f32 y, f32 radX, f32 radY)
{
	D2D->FillEllipse(x, y, radX, radY);
}

void KRenderInterface_D3D11::HUD_End()
{
	D2D->End();
}

void* KRenderInterface_D3D11::HUD_LoadFont(const class KString& name, f32 size, EFontUsage usage, bool bold, bool italic)
{
	return D2D->LoadFont(name, size, usage, bold, italic);
}

UPtr<class KHudBitmap> KRenderInterface_D3D11::HUD_LoadImage(const class KString& name)
{
	UPtr<KHudBitmap_D3D11> bitmap = std::make_unique<KHudBitmap_D3D11>();
	D2D->LoadBitmapFromFile(name, bitmap.get());
	return bitmap;
}

u32 KRenderInterface_D3D11::HUD_CreateTextLayout(const KString& text, EFontUsage font, f32 maxW /*= 0*/, f32 maxH /*= 0*/, u32 existingHandle /*= 0*/)
{
	return D2D->CreateTextLayout(text, font, maxW, maxH, existingHandle);
}

void KRenderInterface_D3D11::HUD_DrawTextLayout(u32 handle, KHudPointF origin)
{
	D2D->DrawTextLayout(handle, origin);
}

void KRenderInterface_D3D11::HUD_SetTextLayoutFontSize(u32 handle, f32 size, i32 startPosition /*= 0*/, i32 length /*= -1*/)
{
	D2D->SetTextLayoutFontSize(handle, size, startPosition, length);
}

void KRenderInterface_D3D11::HUD_DeleteTextLayout(u32 handle)
{
	D2D->DeleteTextLayout(handle);
}

f32 KRenderInterface_D3D11::HUD_GetFontSize(u32 handle)
{
	return D2D->GetFontSize(handle);
}

f32 KRenderInterface_D3D11::HUD_GetFontSize(EFontUsage font)
{
	return D2D->GetFontSize(font);
}

EFontUsage KRenderInterface_D3D11::HUD_GetFontFromHandle(u32 handle)
{
	return D2D->GetFontFromHandle(handle);
}

u32 KRenderInterface_D3D11::HUD_GetTextLineCount(u32 handle)
{
	if (!D2D->Layouts.contains(handle)) return 0;
	return D2D->Layouts[handle].LineCount;
}

f32 KRenderInterface_D3D11::HUD_GetTextHeight(u32 handle)
{
	if (!D2D->Layouts.contains(handle)) return 0;
	D2D->UpdateTextLayout(D2D->Layouts[handle]);
	return D2D->Layouts[handle].TotalHeight;
}

f32 KRenderInterface_D3D11::HUD_GetTextWidth(u32 handle)
{
	if (!D2D->Layouts.contains(handle)) return 0;
	D2D->UpdateTextLayout(D2D->Layouts[handle]);
	return D2D->Layouts[handle].TotalWidth;
}

void KRenderInterface_D3D11::HUD_DrawBitmap(class KHudBitmap* bmp, f32 x, f32 y, f32 w, f32 h, f32 alpha)
{
	D2D->DrawBitmap(dynamic_cast<KHudBitmap_D3D11*>(bmp), x, y, w, h, alpha);
}

void KRenderInterface_D3D11::HUD_DrawCircleProgressBar(f32 centerX, f32 centerY, f32 radius, f32 progress)
{
	D2D->DrawCircleProgressBar(centerX, centerY, radius, progress);
}

void KRenderInterface_D3D11::HUD_SetRadialBrushForPowerup(u8 index, f32 alpha /*= 1*/)
{
	D2D->PowerupRadialGradients[index]->SetOpacity(alpha);
	D2D->BindRadialBrush(D2D->PowerupRadialGradients[index].Get());
}

void KRenderInterface_D3D11::HUD_SetBitmapBrush(class KHudBitmap* bmp, f32 xpos, f32 ypos, f32 xscale, f32 yscale, f32 alpha, bool repeat)
{
	dynamic_cast<KHudBitmap_D3D11*>(bmp)->UpdateScale(bmp->GetWidth() * xscale, bmp->GetHeight() * yscale);
	D2D->BindBitmapBrush(bmp, xpos, ypos, xscale, yscale, alpha, repeat);
}

void KRenderInterface_D3D11::HUD_SetLinearGradientBrush(u8 index)
{
	D2D->SetLinearGradientBrush(index);
}

void KRenderInterface_D3D11::HUD_LoadFontsFromWad()
{
	D2D->LoadFontsFromWad();
}

void* KRenderInterface_D3D11::HUD_GetFont(EFontUsage usage)
{
	return D2D->GetFont(usage);
}

TVector<IDXGIAdapter*> KRenderInterface_D3D11::EnumerateAdapters()
{
	IDXGIAdapter* adapter;
	std::vector <IDXGIAdapter*> adapters;
	IDXGIFactory* factory = NULL;

	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) return adapters;

	for (u32 i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
		adapters.push_back(adapter);

	if (factory) factory->Release();

	return adapters;
}

void KRenderInterface_D3D11::CreateBitmapScaler(ComPtr<struct IWICBitmapScaler>& scaler)
{
	D2D->CreateBitmapScaler(scaler);
}

void KRenderInterface_D3D11::CreateBitmapFromWicBitmap(struct IWICBitmapScaler* scaler, class KHudBitmap_D3D11* bmp)
{
	D2D->CreateBitmapFromWicBitmap(scaler, bmp);
}

#endif
