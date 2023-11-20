#if !_SERVER && _WIN32

#include "d3d11_render_target.h"
#include "engine/utility/k_assert.h"

void KRenderTarget_D3D11::Create(u32 w, u32 h, FRenderTargetCreationFlags flags)
{
	K_ASSERT(!bCreated, "cannot re-create a render target");

	if (w < 1) w = 1;
	if (h < 1) h = 1;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// initialize texture
	{
		textureDesc.Width = w;
		textureDesc.Height = h;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = 
			flags.Is32Bit() ? DXGI_FORMAT_R32G32B32A32_FLOAT :
			flags.IsSingleFloat() ? DXGI_FORMAT_R32_FLOAT :
			flags.IsDoubleFloat() ? DXGI_FORMAT_R32G32_FLOAT :
			flags.IsSignedNorm() ? DXGI_FORMAT_R8G8B8A8_SNORM :
			DXGI_FORMAT_R8G8B8A8_UNORM;
			//DXGI_FORMAT_R11G11B10_FLOAT;

		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = flags.IsStaging() ? D3D11_USAGE_STAGING 
			: (flags.IsDynamic() ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);

		textureDesc.BindFlags = flags.IsStaging() ? 0 
			: (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE 
			| (flags.IsUnorderedAccess() ? D3D11_BIND_UNORDERED_ACCESS : 0));

		textureDesc.CPUAccessFlags = flags.IsStaging() ? D3D11_CPU_ACCESS_READ : 0;
		textureDesc.MiscFlags = 0;

		GetDevice()->CreateTexture2D(&textureDesc, NULL, &RenderTexture);
	}

	// staging targets will not be bound to the gpu
	// instead they can be used to feed data from gpu resources to the cpu
	if (!flags.IsStaging())
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
		ZeroMemory(&renderTargetDesc, sizeof(renderTargetDesc));
		renderTargetDesc.Format = textureDesc.Format;
		renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetDesc.Texture2D.MipSlice = 0;
		GetDevice()->CreateRenderTargetView(RenderTexture.Get(), &renderTargetDesc, &RenderTarget);

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;
		ZeroMemory(&shaderResourceDesc, sizeof(shaderResourceDesc));
		shaderResourceDesc.Format = textureDesc.Format;
		shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceDesc.Texture2D.MipLevels = 1;
		GetDevice()->CreateShaderResourceView(RenderTexture.Get(), &shaderResourceDesc, &RenderShaderResource);
	}

	if (flags.IsUnorderedAccess())
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = textureDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.NumElements = 0;
		GetDevice()->CreateUnorderedAccessView(RenderTexture.Get(), &uavDesc, &UnorderedAccessView);
	}

	CreationFlags = flags;
	Width = w;
	Height = h;
}

void KRenderTarget_D3D11::Clear(f32 r /*= .1*/, f32 g /*= .1*/, f32 b /*= .1*/, f32 a /*= 1*/)
{
	K_ASSERT(RenderTarget.Get(), "cannot clear invalid render target");

	float color[] = { r, g, b, a };
	GetContext()->ClearRenderTargetView(RenderTarget.Get(), color);
}


UPtr<class KMappedRender> KRenderTarget_D3D11::GetMapped()
{
	// we will copy TO this target
	FRenderTargetCreationFlags flags;
	flags.EnableStaging();
	UPtr<KRenderTarget_D3D11> target = std::make_unique<KRenderTarget_D3D11>();
	target->Create(Width, Height, flags);

	ID3D11Resource* src;
	RenderShaderResource->GetResource(&src);

	ID3D11Resource* dst = target->RenderTexture.Get();
	GetContext()->CopyResource(dst, src);
	
	// data has now been copied from this target to the new target
	// map the new target for cpu read

	D3D11_MAPPED_SUBRESOURCE mapped;
	ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
	GetContext()->Map(dst, 0, D3D11_MAP_READ, 0, &mapped);

	UPtr<u8[]> data = std::make_unique<u8[]>(Width * Height * 4);
	memcpy(data.get(), mapped.pData, Width * Height * 4);
	GetContext()->Unmap(dst, 0);

	return std::make_unique<KMappedRender_D3D11>(std::move(data), mapped.RowPitch, mapped.DepthPitch, Width, Height);
}

void KRenderTarget_D3D11::BindTexture2D(u8 slot, EShaderStage stage /*= EShaderStage::Pixel*/)
{
	K_ASSERT(!CreationFlags.IsStaging(), "cannot bind a render target as a shader resource that was created with the Staging flag");

	switch (stage)
	{
		case EShaderStage::Pipeline:
			GetContext()->PSSetShaderResources(slot, 1, RenderShaderResource.GetAddressOf());
			GetContext()->VSSetShaderResources(slot, 1, RenderShaderResource.GetAddressOf());
			break;
		case EShaderStage::Pixel:
			GetContext()->PSSetShaderResources(slot, 1, RenderShaderResource.GetAddressOf());
			break;
		case EShaderStage::Vertex:
			GetContext()->VSSetShaderResources(slot, 1, RenderShaderResource.GetAddressOf());
			break;
		case EShaderStage::Compute:
			GetContext()->CSSetShaderResources(slot, 1, RenderShaderResource.GetAddressOf());
			break;
	}

	
}

void* KRenderTarget_D3D11::GetUAV()
{
	return UnorderedAccessView.Get();
}

FColor8 KMappedRender_D3D11::GetAt(u32 x, u32 y) const
{
	u8 r = Data.get()[(y * RowPitch) + (x * 4) + 0];
	u8 g = Data.get()[(y * RowPitch) + (x * 4) + 1];
	u8 b = Data.get()[(y * RowPitch) + (x * 4) + 2];
	u8 a = Data.get()[(y * RowPitch) + (x * 4) + 3];

	return { r, g, b, a };
}

#endif