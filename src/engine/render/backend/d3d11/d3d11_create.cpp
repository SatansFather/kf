// declare this in its own document so we can include less in KRenderInterface cpp file
// unique ptr forward declares as class members must be included where the object is created

#if !_SERVER && _WIN32

#include "engine/render/interface/render_interface.h"
#include "d3d11_interface.h"

// forward declared unique pointers in KRenderInterface_D3D11
#include "d3d11_render_target.h"
#include "engine/render/interface/window.h"
#include "engine/render/interface/rasterizer.h"

std::unique_ptr<KRenderInterface> KRenderInterface::CreateD3D11(bool new_thread)
{
	auto i = std::make_unique<KRenderInterface_D3D11>(new_thread);
	i->D3D11Interface = i.get();
	return i;
}

#endif
