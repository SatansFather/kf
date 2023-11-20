#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "../ent_prop.h"
#include "engine/render/bufferslots.h"
#include "engine/system/time.h"
#include <mutex>

struct KBufferUpdateResult
{
	enum 
	{
		Changed = 1,
		Visible = 2,
	};

	u8 Flags = 255;

	KBufferUpdateResult() = default;
	KBufferUpdateResult(bool val)
	{
		SetChanged(val);
		SetVisible(val);
	};
	KBufferUpdateResult(bool changed, bool visible)
	{
		SetChanged(changed);
		SetVisible(visible);
	}

	bool IsChanged() const { return Flags & Changed; }
	bool IsCulled() const { return !(Flags & Visible); }

	void SetChanged(bool change)
	{
		if (!change) Flags &= ~Changed;
		else Flags |= Changed;
	}

	void SetVisible(bool vis)
	{
		if (!vis) Flags &= ~Visible;
		else Flags |= Visible;
	}	
};

class KRenderablePropBase
{
public:

	// if not negative, will live this long without needing an entity tick function
	f32 RenderableLifespan = -1;
	GVec3 LastFrameRenderPosition;

	virtual void AllocateRenderBuffer(u32 count) = 0;
	virtual void UpdateRenderableBuffers(bool force) = 0;
	virtual void SetNextForceUpdateFrame(u32 frame) = 0;

	virtual void InitializeRenderable() {}
};

KRenderBufferInfo& GetInfoFromGame(const std::type_info* t);
KRenderBufferInfo& GetInfoFromRender(const std::type_info* t);

void AddInfoToRenderArray(KRenderBufferInfo* info);

template <typename T>
void FlagBufferForSkip(T& t)
{
	GetInfoFromGame(&typeid(t)).bSkipNext = true;
}

template <typename T>
void FlagBufferForViewWeapon(T& t)
{
	GetInfoFromGame(&typeid(t)).bNextIsViewWeapon = true;
}

void CreateBuffersInRenderThread(std::function<void()> func);

// each renderable entity is expected to have a different buffer type
template <typename... TArgs>
class KEntProp_Renderable : public KEntProp, public KRenderablePropBase
{
	enum 
	{
		Visible_Culling = 1,	// true if object is not culled
		Visible_Configured = 2,	// true if object is supposed to be seen
		Visible_LastFrame = 4,	// true if object was visible last frame
	};
	u8 RenderableFlags = 255;

	// very large pools will run in multiple threads
	static std::mutex UpdateMutex;
	static u32 NextForceUpdateFrame;

	bool RenderableVisibleLastFrame() const { return RenderableFlags & Visible_LastFrame; }
	bool RenderableIsCulled() const { return !(RenderableFlags & Visible_Culling); }
	void RenderableSetCulled(bool culled)
	{
		if (culled) RenderableFlags &= ~Visible_Culling;
		else RenderableFlags |= Visible_Culling;
	}

	// returns true if the object is configured to be visible and is not culled
	bool RenderableIsReallyVisible() const { return IsVisible() && !RenderableIsCulled(); }

protected:
	
	KEntProp_Renderable()
	{
		// let the buffer know something new is here
		MarkInfoChanged();
	}

	virtual ~KEntProp_Renderable()
	{
		// render is always a frame behind
		// we need to update buffers to be seen after dying
		//MarkInfoChanged();
		//UpdateRenderableBuffers(GetEntity()->GetFrameCreated() <= KTime::FrameCount() + 2, true);
	}

	virtual KBufferUpdateResult UpdateBuffers(TArgs&... args) = 0;

	// tell the master buffer something has changed
	void MarkInfoChanged()
	{
		TTuple<TArgs...> items;

		TUPLE_FOR_EACH(items, TArgs, t, [],
			KRenderBufferInfo& info = GetInfoFromGame(&typeid(t));
			info.bChanged = true;
		);
	}

	void UpdateRenderableBuffers(bool force) final
	{
		TTuple<TArgs...> items;

		// this variable will determine if the entire buffer will need to be updated
		// if any object moves, spawns, despawns, or updates visibility, update is required
		bool change = false;

		if (IsVisible())
		{
			// update buffers on child object if this is allowed to be visible
			KBufferUpdateResult result = std::apply(std::bind_front(&KEntProp_Renderable<TArgs...>::UpdateBuffers, this), items);
			change = result.IsChanged() || force;

			RenderableSetCulled(result.IsCulled());
		}

		bool vis = RenderableIsReallyVisible();

		// if visibility changed then the buffer will shift, and needs to be updated
		bool visChange = vis != RenderableVisibleLastFrame();
		change = change || visChange;

		// store visibility for next frame
		if (vis) RenderableFlags |= Visible_LastFrame;
		else	 RenderableFlags &= ~Visible_LastFrame;

		// right now the TUPLE_FOR_EACH macro doesnt allow a comma in the capture brackets
		u8 capture = 0;
		if (change) capture |= 1;
		if (vis) capture |= 2;

		// iterate tuple, copy items into their respective buffer
		TUPLE_FOR_EACH(items, TArgs, t, [capture],
			KRenderBufferInfo& info = GetInfoFromGame(&typeid(t));
			if (info.LastFrameUpdated != KTime::FrameCount() && !info.bNextIsViewWeapon)
			{
				info.LastFrameUpdated = KTime::FrameCount();
				info.ActiveCount = 0; // new frame, starting over

				if (info.LastFrameUpdated == NextForceUpdateFrame)
					info.bChanged = true;
			}

			// if any item in the buffer changes, flag the whole thing to update
			if (capture & 1) info.bChanged = true;
			
			if ((capture & 2) && !info.bSkipNext) // visible
			{
				if (info.bNextIsViewWeapon)
				{
					memcpy(info.Data_ViewWeapon.get(), &t, sizeof(t));
					info.bPendingViewWeapon = true;
				}
				else
				{
					if (info.ActiveCount < info.MaxCount)
					{
						// even if nothing in the buffer changed yet, if this is visible then we need it
						// incrementing the active count ensures this keeps the same place in the buffer from the last frame
						//	if info.bChanged is not true yet
						info.ActiveCount++; 

						if (info.bChanged) memcpy(info.Data.get() + (info.ActiveCount - 1) * sizeof(t), &t, sizeof(t));
					}
				}
			}

			info.bSkipNext = false;
			info.bNextIsViewWeapon = false;
		);
	}

	void AllocateRenderBuffer(u32 count) override
	{
		TTuple<TArgs...> items;

		TUPLE_FOR_EACH(items, TArgs, t, [count],
			KRenderBufferInfo& info = GetInfoFromGame(&typeid(t));

			info.MaxCount += count;
			info.TypeSize = sizeof(t);
			info.Data.reset();
			info.Data = std::make_unique<u8[]>(info.MaxCount * sizeof(t));
			info.AllowUpdateFunction = &t.ShouldBeUpdated;

			// need info from render interface instead of game instance
			KRenderBufferInfo& rInfo = GetInfoFromRender(&typeid(t));
			rInfo.RenderFunction = &t.RenderBuffer;

			// buffers should be created in render thread for safety
			u32 typeSize = info.TypeSize;
			u32 maxCount = info.MaxCount;
			const auto callCreateFunc = [t, &rInfo, typeSize, maxCount]() mutable -> void
			{
				t.CreateBuffers(rInfo, typeSize, maxCount);
			};
			CreateBuffersInRenderThread(callCreateFunc);

			AddInfoToRenderArray(&rInfo);

			if (rInfo.GpuBuffer_ViewWeapon && !info.Data_ViewWeapon)
				info.Data_ViewWeapon = std::make_unique<u8[]>(sizeof(t));

			info.ExtraUpdateFunction = rInfo.ExtraUpdateFunction;
		);
	}

	void SetNextForceUpdateFrame(u32 frame) override
	{
		NextForceUpdateFrame = frame;
	}

public:

	bool IsVisible() const { return RenderableFlags & Visible_Configured; }
	void SetVisible(bool vis) 
	{ 
		if (vis) RenderableFlags |= Visible_Configured;
		else RenderableFlags &= ~Visible_Configured;
	}
};


template <typename... TArgs>
u32 KEntProp_Renderable<TArgs...>::NextForceUpdateFrame = 0;

template <typename... TArgs>
std::mutex KEntProp_Renderable<TArgs...>::UpdateMutex;

#endif