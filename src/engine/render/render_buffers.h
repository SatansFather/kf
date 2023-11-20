#pragma once

#if !_SERVER

#include "render_buffer_info.h"

struct KRenderBuffers
{
	KBufferMap Map;

	TVector<KRenderBufferInfo*> OpaqueBuffers;
	TVector<KRenderBufferInfo*> TransparentBuffers;

	TVector<KRenderBufferInfo*> ViewWeaponOpaqueBuffers;
	TVector<KRenderBufferInfo*> ViewWeaponTransparentBuffers;
};

#endif