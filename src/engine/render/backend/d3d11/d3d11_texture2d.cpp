#if !_SERVER && _WIN32

#include "d3d11_texture2d.h"
#include "engine/utility/k_assert.h"

void KTexture2D_D3D11::Finalize(const D3D11_TEXTURE2D_DESC& td, const D3D11_SUBRESOURCE_DATA& sd)
{
	ComPtr<ID3D11Texture2D> tex;
	GetDevice()->CreateTexture2D(&td, &sd, &tex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv = {};
	srv.Format = td.Format;
	srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv.Texture2D.MostDetailedMip = 0;
	srv.Texture2D.MipLevels = 1;
	
	K_ASSERT_HR(GetDevice()->CreateShaderResourceView(tex.Get(), &srv, &TextureResource),
		"could not create shader resource view for Texture2D");
}

void KTexture2D_D3D11::CreateFromSurface(UPtr<KSurface2D>& surface)
{
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = surface->GetPaddedWidth();
	td.Height = surface->GetPaddedHeight();
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = surface->GetData();
	sd.SysMemPitch = surface->GetPaddedWidth() * sizeof(FColor8);

	Finalize(td, sd);

	Surface = std::move(surface);
}
	
void KTexture2D_D3D11::CreateFrom2dFloatArray(f32* values, u32 w, u32 h)
{
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = values;
	sd.SysMemPitch = w * sizeof(f32);

	Finalize(td, sd);
}

void KTexture2D_D3D11::CreateFromFreetypeGlyph(struct FT_GlyphSlotRec_* glyph)
{
	/*D3D11_TEXTURE2D_DESC td = {};
	td.Width = glyph->bitmap.width;
	td.Height = glyph->bitmap.rows;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = glyph->bitmap.buffer;
	sd.SysMemPitch = glyph->bitmap.pitch;

	Finalize(td, sd);*/
}

#endif

