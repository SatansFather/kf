#if !_SERVER

#include "renderable.h"
#include "engine/game_instance.h"
#include "engine/render/render_buffer_info.h"
#include "engine/render/interface/render_interface.h"

KRenderBufferInfo& GetInfoFromGame(const std::type_info* t)
{
	return KGameInstance::Get().RenderBufferMap[t];
}

KRenderBufferInfo& GetInfoFromRender(const std::type_info* t)
{
	return GetRenderInterface()->RenderBufferPointers.Map[t];
}

void AddInfoToRenderArray(KRenderBufferInfo* info)
{
	if (info->bTransparent)
		GetRenderInterface()->RenderBufferPointers.TransparentBuffers.push_back(info);
	else
		GetRenderInterface()->RenderBufferPointers.OpaqueBuffers.push_back(info);
}

void CreateBuffersInRenderThread(std::function<void()> func)
{
	KRenderInterface* iface = GetRenderInterface();
	while (iface->bPendingBufferFunc) {}
	iface->BufferMutex.lock();
	iface->CreateBufferFunc = func;
	iface->bPendingBufferFunc = true;
	iface->BufferMutex.unlock();
	while (iface->bPendingBufferFunc) {}
}

#endif