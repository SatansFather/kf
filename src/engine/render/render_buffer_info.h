#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "interface/buffers/gpu_buffer.h"
#include "interface/buffers/vertex_buffer.h"


struct KRenderBufferInfo
{
	UPtr<u8[]> Data;
	UPtr<u8[]> Data_ViewWeapon;
	u32 MaxCount = 0;
	u32 ActiveCount = 0;
	u32 TypeSize = 0;
	u32 LastFrameUpdated = MAX_U32;
	std::function<void(const KRenderBufferInfo&)> RenderFunction;
	std::function<bool(const KRenderBufferInfo&)> AllowUpdateFunction;
	std::function<void(bool, bool)> ExtraUpdateFunction; // bool param true if going to gpu
	UPtr<KGpuBuffer> GpuBuffer;
	UPtr<KGpuBuffer> GpuBuffer_ViewWeapon;
	UPtr<KVertexBuffer> VertexBuffer;
	bool bAllowUpdate = false;
	bool bChanged = false; // buffer size or individual object changed
	//bool bPostWater = false;
	bool bTransparent = false;

	// call from UpdateBuffers to flag as invisible
	// useful when UpdateBuffers takes multiple args and not all should be visible
	bool bSkipNext = false; 

	// flagged when the next entry will be the player's viewed weapon
	bool bNextIsViewWeapon = false;

	// this buffer has a view weapon ready to render this frame
	bool bPendingViewWeapon = false;

	// currently drawing view weapon
	bool bDrawingViewWeapon = false;

	typedef TMap<const std::type_info*, struct KRenderBufferInfo> BufferMap;
	static void CopyMap(BufferMap* from, BufferMap* to, bool initial = false, bool gpu = false);
};

typedef KRenderBufferInfo::BufferMap KBufferMap;

#endif