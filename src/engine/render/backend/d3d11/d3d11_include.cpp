#if !_SERVER && _WIN32

#include "d3d11_interface.h"

ID3D11DeviceContext* D3D11Object::GetContext() const
{
	return ImmediateContext;
}

ID3D11Device* D3D11Object::GetDevice() const
{
	return Device;
}

D3D11Object::D3D11Object()
{
	auto i = GetD3D11Interface();
	ImmediateContext = i->GetImmediateContext();
	Device = i->GetDevice();
}

#endif
