#pragma once

#include "entity.h"
#include "properties/killable.h"
#include "properties/controllable.h"
#include "properties/collidable.h"
#include "properties/movable.h"
#include "mover_info.h"
#include "engine/collision/hit_result.h"
#include "engine/net/snapshottable.h"
#include "properties/net_pos.h"
	
class KEntity_Character : public KEntity,
	public KEntProp_Killable,
	public KEntProp_Controllable,
	public KEntProp_CollidableBBox,
	public KEntProp_Movable,
	public KEntProp_NetPosition,
	public KSnapshottable
{
public:

	SNAP_PROP(GVec3, PositionForOwner, SNAP_SEND_OWNER)
	SNAP_PROP(u8, KillingPlayerIndex = NULL_PLAYER, SNAP_DESTROY)

	SNAP_PROP_TRANSIENT(KNetVec3, ReplayPosition, SNAP_REPLAY_ONLY, SNAP_SEND_OWNER)
	void SetTransient_ReplayPosition(KNetVec3& val);
	void GetTransient_ReplayPosition(KNetVec3& val);

protected:

#if 0
	KMovementProperties WalkingMovement;
	KMovementProperties	FlyingMovement;
	KMovementProperties SwimmingMovement;
#endif

	/*struct
	{
		GVec3 Velocity;	

		f32 AirAcceleration = 0;
		f32 JumpHeight = 40;
		f32 GravityScale = 1;
		GVec3 FloorNorm;
		u32 RemainingPushFrames = 0;
		f32 PushAccelScale = .1;
		EMoveState State;
		KMovementProperties Props;
		TObjRef<KEntity> Base; // can be a floor or a volume
		GVec3 LastFrameVelocity;
		GVec3 LastFrameStart;
		
	} MoveState;*/
	f32 HeadBob = 0;

	GVec3 FacingDirection;
	f64 LastStopTime;
	GFlt LastStepWave = 0;
	GFlt CameraAdjustZ = 0;

public:

	SNAP_PROP(u16, KillStreak = 0, SNAP_DESTROY)

	SNAP_PROP(f32, DamageMultiplier = 1)

private:

	void UpdateInputVector();
	virtual GVec3 GetAiInputvector();

public:

	void InitNetObject() override;
	void OnNetUpdate() override;
	void PreCreateSnapshot() override;

	void Tick() override final;

	void OnEnteredWater(const GHitResult& hit) override;
	bool IsCollisionEnabled() override;

	void OnKilled(class KNetPlayer* player = nullptr) override;

	virtual void CharacterTick() {}

	GVec3 GetGibPushVelocity(const GVec3& otherVel) override;

	bool ForceMaintainPortalRotation() const override { return false; }

	bool CanTelefrag() override { return true; }
	bool CanBeTelefragged() override { return true; }
};