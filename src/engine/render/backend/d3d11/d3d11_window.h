#pragma once

#if !_SERVER && _WIN32

#include <memory>
#include "engine/render/interface/window.h"
#include "engine/os/windows/windows.h"

class KGameWindow_D3D11 : public KGameWindow
{
	friend class KRenderInterface_D3D11;

private:

	static UPtr<KGameWindow_D3D11> CreateGameWindowD3D(u32 resX, u32 resY, u8 state);

public:

	void OnResize() override;

};

#endif // _WIN32
