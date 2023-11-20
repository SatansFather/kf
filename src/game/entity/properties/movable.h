#pragma once

#include "../ent_prop.h"
#include "../mover_info.h"
#include "move_states.h"
#include "engine/collision/hit_result.h"
#include "game/saved_move.h"
#include "engine/net/snapshottable.h"
#include "net_pos.h"

#define CROUCH_FRAMES 12 // cannot be less than 2


struct KTestMovementResult
{
	
};

class KEntProp_Movable : public KEntProp
{
private:

	struct MoveHitNormals
	{
		GVec3 HitNorms[8];
		GVec3 RepeatNorms[2];
		u8 NormCount = 0;
		u8 RepeatCount = 0;
		struct 
		{
			bool AddedNorm = false;
			bool AddedRepeat = false;
		} Resolve;
		void ResetResolve();
		void RevertResolve();
		bool Update(const GVec3& norm, KEntProp_Movable* mover);
	};

	friend struct MoveHitNormals;
	
protected:

public:

	enum EMovableFlags
	{
		MF_TestingStepUp	= 1,
		MF_TinyStep			= 2,
		MF_PendingMoveBlock = 4,
		MF_Enabled			= 8,
		MF_Teleported		= 16,
		MF_GroundTrace		= 32,
		MF_InsideFluid		= 64,
		MF_IsSimpleMovement = 128,
		MF_SkipGroundTrace	= 256,
		MF_IsCrouching		= 512,
		
		MF_ReplayingMoves	= 2048,
		MF_BlockOnContact	= 4096,
		MF_CanBePushed		= 8192,
	};


	u16 MovableFlags = MF_CanBePushed;

	u8 MaxStepHeight = 2;
	GFlt GravityScale = 1;

private:

	// store blocked move data when stepping until it is confirmed successful
	struct
	{
		GHitResult Hit;
		GVec3 PreVelocity;
	} PendingMoveBlock;

	TObjRef<KEntity> MovementBase; // can be a floor or a volume
	KMovementProperties MovementProperties;
public:
	SNAP_PROP(EMoveState, MovementState, SNAP_SEND_OTHERS)
private:
	GFlt LastMoveStepDistance = 0;

	TVector<void*> ThisFrameHitEntities;

	GFlt LastMoveAlpha = 1;

	f32 WaterPenetration = -1;
	f32 LastWaterPenetration = -1;

public:

	SNAP_PROP(u8, CrouchDepth = 0, SNAP_SEND_OTHERS)

	TMap<u32, KSavedMove> SavedMoves;

	TObjRef<KEntity> IgnoreEntity; // usually projectile owner

	// movement speed and direction, walking ignores z
	// value used for movement updates
	SNAP_PROP(GVec3, VelocityForOwner, SNAP_SEND_OWNER)
	GVec3 Velocity;

	SNAP_PROP(u8, PushFramesRemaining = 0, SNAP_SEND_OWNER)


	GVec3 LastFrameVelocity = 0;

	GFlt PushAccelScale = .1;
	GFlt PushDecelScale = .1;
	GVec3 PendingImpulse;

	u32 LastLandFrame = 0;
protected:
	f32 LandZ = 0;
public:

	// how many frames must pass in between moves
	u8 FrameTickInterval = 1;
	u8 FramesSinceMove = 0;

	// frames of ping comp
	u8 CompFrames = 0;

	KEntProp_Movable();

public:
	
	virtual void OnMoveBlocked(const GVec3& preVel, const GHitResult& hit) {}
	virtual void OnEnteredWater(const GHitResult& hit) {}

	bool OnOverlapChannel(u32 channel, const GVec3& preVel, GHitResult& hit);

	virtual bool OnOverlapWorldStatic(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapLight(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapPrecipitation(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapPortal(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapLauncher(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapWater(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapDamage(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapPlayerCharacter(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapMonsterCharacter(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapGib(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapPickup(const GVec3& preVel, GHitResult& hit);
	virtual bool OnOverlapWeapon(const GVec3& preVel, GHitResult& hit);

private:

	void OnMoveOverlap(const GVec3& preVel, const GHitResult& hit, u32 channel) {}

	void Movable_SlideOnLanding(const GVec3& norm);

	bool Movable_MoveTrace(const GVec3& start, const GVec3& finish, GHitResult& hit, u8& resolves, KTraceCondition condition = nullptr);

	// separate properties for water and air
	void Movable_UpdatePhysicsMovementProperties();

	// falling entities have different step rules
	GFlt Movable_GetMaxStepHeight();

protected:

	// traces a move against solid collision as well as potential volume overlaps
	// returns false if the owning entity was killed or destroyed and should stop the move
	//bool MoveTrace(const GVec3& start, const GVec3& end, GHitResult& hit);

	// gets collision component, if the entity has one
	class KEntProp_CollidableBBox* Movable_GetCollision();

	bool Movable_Bounce(const GVec3& norm);

	// directs a vector along a wall with the given normal, clipping its speed
	void ClipVector(const GVec3& in, const GVec3& norm, GVec3& out);

	// redirects floor velocity when walking on slopes
	GVec3 RedirectAlongFloor(const GVec3& trace);

	// trace to the ground to see if we're on it
	// return false if the entity is dead
	bool Movable_GroundCheck();

	// adjusts and responds to the input vector based on current state
	GVec3 Movable_ProcessInput(GVec3 input);

	// updates the velocity from the given input vector
	void Movable_UpdateVelocity(const GVec3& input);

	void Movable_PhysicsMove(const GVec3& input);

	// if the entity has collision, applies velocity to collision and adjusts move accordingly
	GFlt Movable_MoveAgainstCollision(GFlt remainingMove = 0);

	i32 Movable_StepUp(const GHitResult& hit, GFlt& remaining, MoveHitNormals& moveHits);

	void Movable_DoJump();
	void Movable_UpdateCrouchState(bool wantsCrouch);
	bool Movable_CanExitCrouch();

	KMoveState_Walking*		GetWalkingMovement();
	KMoveState_Swimming*	GetSwimmingMovement();
	KMoveState_Flying*		GetFlyingMovement();
	KMoveState_Physics*		GetPhysicsMovement();

	// gets the movement properties for the current move state
	KMovementProperties* GetMovementProperties();

	KMovementSurfaceModifier GetMovementSurfaceMods() const;

	void SetReplayingMoves(bool replaying);

public:

	void SetMovementState(EMoveState state);
	
	bool IsWalkable(const GVec3& norm);

	bool IsWalking() const { return MovementState == EMoveState::Walking; }
	bool IsFalling() const { return MovementState == EMoveState::Falling; }
	bool IsSwimming() const { return MovementState == EMoveState::Swimming; }
	bool IsFlying() const { return MovementState == EMoveState::Flying; }
	bool IsPhysics() const { return MovementState == EMoveState::Physics; }
	bool IsGroundMovement() const { return IsWalking() || IsFalling(); }
	bool IsCrouching() const;

	bool IsInWater() { return MovableFlags & MF_InsideFluid; }

	void SetMovementEnabled(bool enabled);
	bool MovementIsEnabled() const { return MovableFlags & MF_Enabled; }

	void AddPendingImpulse(const GVec3& vel);

	GFlt GetJumpVelocity(class KMoveState_Walking* walk = nullptr);
	GFlt GetGravityZ();
	GFlt GetLastMoveStepDistance() { return LastMoveStepDistance; }
	f32 GetCrouchDepth() const;
	bool ShouldMoveThisFrame();

	GFlt GetLastMoveAlpha() const { return LastMoveAlpha; }

	GVec3 GetPendingImpulse() const { return PendingImpulse; }

	f32 GetWaterPenetration() const { return WaterPenetration; }

	bool TeleportedThisFrame() const { return MovableFlags & MF_Teleported; }

	u32 GetLastLandFrame() const { return LastLandFrame; }

	void Push(GVec3 impulse, u32 pushFrames = 5);

	void SetSkipGroundCheck(bool skip);
	void SetSimpleMovement(bool simple);
	void SetBlockOnContact(bool block);

	bool ShouldSkipGroundTrace() const { return MovableFlags & MF_SkipGroundTrace; }
	bool IsUsingSimpleMovement() const { return MovableFlags & MF_IsSimpleMovement; }

	bool PerformMovement(const GVec3& input);
	KTestMovementResult TestMovement();

	void SetCanBePushed(bool v);
	bool CanBePushed() const { return MovableFlags & MF_CanBePushed; }

	virtual void ExplodePush(const GVec3& dir, f32 strength);

	// velocity to apply to a physics object when hitting it
	virtual GVec3 GetItemPushVelocity(const GVec3& otherVel) { return Velocity; }
	virtual GVec3 GetGibPushVelocity(const GVec3& otherVel) { return Velocity; }
	
	void CreateSavedMove();
	void CheckServerMove(u32 frame, const GVec3& velocity, const GVec3& position, u16 pitch, u16 yaw);
	bool IsReplayingMoves() const { return MovableFlags & MF_ReplayingMoves; }

	bool ShouldBlockOnContact() const { return MovableFlags & MF_BlockOnContact; }

	virtual bool ForceMaintainPortalRotation() const { return true; }
};