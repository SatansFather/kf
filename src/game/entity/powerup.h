#pragma once

#include "entity.h"
#include "engine/net/snapshottable.h"
#include "powerup_index.h"

class KEntity_Powerup : 
	public KEntity,
	public KSnapshottable
{
	friend class KEntity_Pickup_Powerup;
	
	u32 FrameDuration = std::round(1.0 / GameFrameDelta()) * 30;

protected:

	u32 RemainingFrames = FrameDuration;
	u32 PowerupID = EPowerupID::NumPowerups;

	TObjRef<KEntity> CarryingEntity;
	GVec3 LastFramePosition;

public:

	SNAP_PROP_TRANSIENT(u32, EndFrame)
	void GetTransient_EndFrame(u32& val);
	void SetTransient_EndFrame(u32& val);
	SNAP_PROP(u32, CarryingEntID = 0, SNAP_FIRST_ONLY)

	void InitNetObject() override;

	class KEntProp_PowerupInventory* GetInventory();

	void Tick() override;

	u32 GetRemainingFrames() const { return RemainingFrames; }
	u32 GetFrameDuration() const { return FrameDuration; }
	void AddRemainingFrames(u32 frames) { RemainingFrames += frames; }
	void SetRemainingFrames(u32 frames) { RemainingFrames = frames; }
};