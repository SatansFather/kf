#pragma once

#include "entity.h"
#include "properties/collidable.h"
#include "properties/movable.h"
#include "properties/move_states.h"
#include "engine/system/hashed_poolable.h"
#include "engine/net/snapshottable.h"
#include "properties/net_pos.h"
#include "../../engine/audio/sound_list.h"

struct EReppedPickupFlags
{
	enum
	{
		Spawned  = 1,
		Dropped  = 2,
		HP20     = 4,
		HP50     = 8,
		HP100    = 16,
		HP200    = 32,
	};
};

class KEntity_PickupBase : public KEntity,
	public KEntProp_CollidableBBox,
	public KEntProp_Movable,
	public KMoveState_Physics,
	public KEntProp_NetPosition,
	public KSnapshottable
{

	enum // pickup flags
	{
		PF_IsDropItem		= 1,
		PF_SpawnPerEntity	= 2,
		PF_IsSpawnedGlobal	= 4,
		PF_StartSpawned		= 8,
		PF_DestroyNextTick  = 16,
	};
	
	
	
	std::unordered_map<std::array<u32, 4>, u16, ArrayHasher> RemainingEntityRespawnFrames;

	u16 RemainingGlobalRespawnFrames = 0;

	u8 Flags = PF_StartSpawned;

public:

	SNAP_PROP(u32, LastPickupEntID = 0, SNAP_DESTROY)
	SNAP_PROP(u8, ReppedFlags = 0)

protected:

	f32 LastRenderTime = 0;
	f32 LastZ = 0; // bobbing
	f32 LastRadius = 0; // light
	f32 LastScaleAlpha = 0;
	f32 TotalTime = 0;
	u32 RespawnFrameCount = 0;
	u32 SpawnStateChangeFrame = 0;

public:

	KEntity_PickupBase();

	virtual void OnRep_ReppedFlags();

	void PreCreateSnapshot() override;

	void InitNetObject() override;

	bool IsDropItem() const { return Flags & PF_IsDropItem; }
	bool ShouldStartSpawned() const { return Flags & PF_StartSpawned; }
	bool ShouldSpawnPerEntity() const { return Flags & PF_SpawnPerEntity; }
	bool IsSpawned(KEntity* ent = nullptr) const;

	void SetStartSpawned(bool start);

	void SetDropItem(bool drop);

	// returns true if item was picked up
	bool OnOverlap(KEntity* ent);

	void Tick() override;

	bool IsSpawnedForEntity(KEntity* ent) const;
	bool IsSpawnedForViewedEntity() const;

	void SetGlobalSpawnState(bool spawned);

	virtual bool CanPickUp(KEntity* ent = nullptr) = 0;
	void Despawn(KEntity* ent = nullptr);
	
	void Poolable_OnInstantiated() override;

	void FillLightProperties(struct KDynamicLight& light);

	virtual void PickUp(KEntity* ent) = 0;

	virtual KSoundID GetRespawnSound() const { return KSoundID::Cannon_Explode_1; }
	virtual KSoundID GetDespawnSound() const { return KSoundID::Cannon_Explode_1; }

	virtual KString GetPickupMessage() = 0;

	void TryIncreaseSize();

	void ExplodePush(const GVec3& dir, f32 strength) override;

	void InitDropItem();

	void DestroyNextTick();

	void AddPositionToHistory(u32 frame) override;
};