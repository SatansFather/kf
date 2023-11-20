#if !_SERVER

#include "render_buffer_info.h"
#include "interface/render_interface.h"

void KRenderBufferInfo::CopyMap(BufferMap* from, BufferMap* to, bool initial /*= false*/, bool gpu /*= false*/)
{
	// in args:
	// initial - copying from game instance to render bridge
	// gpu - copying from render bridge to gpu buffer

	for (const auto& kv : *from)
	{
		KRenderBufferInfo& src = (*from)[kv.first];
		if (!src.Data.get()) continue;

		// if the final entry died, ActiveCount never got a chance to update
		if (initial && src.LastFrameUpdated != KTime::FrameCount() && src.ActiveCount > 0)
		{
			src.ActiveCount = 0;
			src.bChanged = true;
		}

		KRenderBufferInfo& dst = (*to)[kv.first];
		dst.ActiveCount = src.ActiveCount;
	
		dst.bChanged = src.bChanged;
		src.bChanged = false; // clear for next frame
		// handle view weapon
		if (src.bPendingViewWeapon)
		{
			dst.bPendingViewWeapon = true;
			src.bPendingViewWeapon = false;

			if (gpu)
			{
				//K_ASSERT(src.Data_ViewWeapon.get(), "render bridge view weapon buffer null");
				//K_ASSERT(dst.GpuBuffer_ViewWeapon.get(), "no view weapon gpu buffer allocated");

				GetRenderInterface()->UpdateBuffer(
					dst.GpuBuffer_ViewWeapon.get(),
					src.Data_ViewWeapon.get(), 1);
			
				if (dst.ExtraUpdateFunction)
					dst.ExtraUpdateFunction(true, true);
			}
			else
			{
				if (!dst.Data_ViewWeapon.get())
					dst.Data_ViewWeapon = std::make_unique<u8[]>(src.TypeSize);

				memcpy(dst.Data_ViewWeapon.get(), src.Data_ViewWeapon.get(), src.TypeSize);
				if (src.ExtraUpdateFunction)
					src.ExtraUpdateFunction(false, true);
			}
		}
		else
		{
			dst.bPendingViewWeapon = false;
		}

		// dont copy if nothing changed
		if (!dst.bChanged) continue;

		if (dst.MaxCount != src.MaxCount || !dst.Data.get())
		{
			dst.MaxCount = src.MaxCount;
			dst.TypeSize = src.TypeSize;
			dst.AllowUpdateFunction = src.AllowUpdateFunction;

			// allocate new buffer
			if (!gpu) dst.Data = std::make_unique<u8[]>(dst.MaxCount * dst.TypeSize);
			
		}

		if (initial) dst.bAllowUpdate = src.AllowUpdateFunction(src);
		else		 dst.bAllowUpdate = src.bAllowUpdate;

		if (dst.bAllowUpdate)
		{
			if (gpu)
			{
				if (dst.ActiveCount > 0)
					GetRenderInterface()->UpdateBuffer(
						dst.GpuBuffer.get(),
						src.Data.get(),
						dst.ActiveCount);

				if (dst.ExtraUpdateFunction)
					dst.ExtraUpdateFunction(true, false);
			}
			else
			{
				memcpy(dst.Data.get(), src.Data.get(), dst.TypeSize * dst.ActiveCount);
				if (src.ExtraUpdateFunction)
					src.ExtraUpdateFunction(false, false);
			}
		}
	}
}

#endif