#pragma once

// headers for all d3d11 files

#if !_SERVER && _WIN32

#include <d3d11.h>

#include <DirectXMath.h>

#include <wrl/internal.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")

// d3d11 interfaces inherit from this for easy device and context access
class D3D11Object
{
	ID3D11DeviceContext* ImmediateContext;
	ID3D11Device* Device;

public:

	ID3D11DeviceContext* GetContext() const;
	ID3D11Device* GetDevice() const;

	D3D11Object();
};

#endif
