#pragma once

#include "entity.h"
#include "engine/math/vec3.h"
#include "properties/movable.h"
#include "properties/move_states.h"
#include "properties/killable.h"
#include "properties/collidable.h"
#include "properties/renderable.h"
#include "engine/net/snapshottable.h"

#if !_SERVER
#include "graphics/shotgun_trail.h"
#endif

enum ENetProjectileType
{
	Standard,
	ServerSweepPre,
	ServerSweepPost,
	ServerCosmetic,
};

struct KProjectileCreationParams
{
	GVec3 Position;
	GVec3 Direction;
	GVec3 RenderOffset;
	GFlt Speed = 0;
	class KEntity_Weapon* FiringWeapon = nullptr;
	u8 OwningPlayerIndex = NULL_PLAYER;
	bool bPrimaryFire = true;
};

class KEntity_Projectile :
	public KEntity,
	public KEntProp_Movable,
	public KMoveState_Physics,
	public KEntProp_Killable,
	public KEntProp_CollidableBBox,
	public KSnapshottable
{
	
public:

	SNAP_PROP(KNetVec3, ReplicatedPosition, SNAP_FIRST_ONLY)
	SNAP_PROP(KNetVec3, ReplicatedVelocity, SNAP_FIRST_ONLY)
	SNAP_PROP(bool, bPrimaryFire = false, SNAP_FIRST_ONLY)

	TObjRef<class KEntity_Weapon> FiringWeapon;

	GVec3 LastRenderOffset;
	GVec3 RenderOffset;
	f32 RenderOffsetAdjustRate = 1.1;
	f32 RandomSeed = 0;
	f32 Lifespan = 10;
	f32 PushScale = 400;
	 
	u16 DirectDamage = 100;
	GFlt PushStrength = 1;

	ENetProjectileType NetType = ENetProjectileType::Standard;

	KEntity_Projectile();
	void OnMoveBlocked(const GVec3& preVel, const GHitResult& hit) final;
	void OnEnteredWater(const GHitResult& hit) override;
	void Tick() override final;

	bool PingSweep(u32 frames);
	bool IsSweepProjectile() const;

	void InitNetObject() override;
	void PreCreateSnapshot() override;

	static void InitBaseProjectile(const KProjectileCreationParams& params, KEntity_Projectile* proj);

	bool IsMyProjectile();
	bool IsMyNetProjectile();
	virtual void OnProjectileHit(const GVec3& preVel, const GHitResult& hit) = 0;
	virtual void ProjectileTick() {} 

	// remove this if we want to store projectiles at some point
	virtual void AddPositionToHistory(u32 frame) override {};

	void SplashDamage(const GHitResult& hit, const GVec3& preVel, GFlt radius, GFlt maxDamage, GFlt splashPushScale = 1, GFlt selfDamageScale = .5);

	bool OnOverlapPlayerCharacter(const GVec3& preVel, GHitResult& hit) override;
	bool OnOverlapMonsterCharacter(const GVec3& preVel, GHitResult& hit) override;

	virtual void OverlapCharacter(const GVec3& preVel, GHitResult& hit);

	GVec3 GetItemPushVelocity(const GVec3& otherVel) override;
	GVec3 GetGibPushVelocity(const GVec3& otherVel) override;

	virtual void PreClientLocalTick(u32 frames) {}
};