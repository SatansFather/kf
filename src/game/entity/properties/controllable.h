#pragma once

#include "../ent_prop.h"
#include "engine/math/vec3.h"
#include "engine/net/snapshottable.h"

class KEntProp_Controllable : public KEntProp
{
public:

	SNAP_PROP_TRANSIENT(u16, ReppedPitch, SNAP_SEND_OTHERS)
	SNAP_PROP_TRANSIENT(u16, ReppedYaw, SNAP_SEND_OTHERS)
	void GetTransient_ReppedPitch(u16& val);
	void SetTransient_ReppedPitch(u16& val);
	void GetTransient_ReppedYaw(u16& val);
	void SetTransient_ReppedYaw(u16& val);

	SNAP_PROP_TRANSIENT(u16, ReplayPitch, SNAP_REPLAY_ONLY, SNAP_SEND_OWNER)
	SNAP_PROP_TRANSIENT(u16, ReplayYaw, SNAP_REPLAY_ONLY, SNAP_SEND_OWNER)
	void GetTransient_ReplayPitch(u16& val);
	void SetTransient_ReplayPitch(u16& val);
	void GetTransient_ReplayYaw(u16& val);
	void SetTransient_ReplayYaw(u16& val);

	SNAP_PROP(u16, SpawnYaw, SNAP_SEND_OWNER, SNAP_FIRST_ONLY)
	//void GetTransient_SpawnYaw(u16& val);
	//void SetTransient_SpawnYaw(u16& val);

protected:

	GFlt Pitch = 0, Yaw = 0;

	i8 ForwardInput = 0;
	i8 RightInput = 0;
	i8 UpInput = 0;

	struct
	{
		u8 Keys = 0;
		u8 WeaponIndex = 0;
	} InputState;


public:


	// value is rotated to world
	GVec3 GetInputVector();

	void SetInputVector(GVec3 v);
	void SetInputFromPlayer();
	void SetInputFromClient();

	GVec3 GetInputVectorForMove();

	GFlt GetYaw() { return Yaw; }
	GFlt GetPitch() { return Pitch; }

	u16 GetYawAsInt() const;
	u16 GetPitchAsInt() const;

	i8 GetForwardInput() const { return ForwardInput; };
	i8 GetRightInput() const { return RightInput; };
	i8 GetUpInput() const { return UpInput; };

	// player, not a bot or ai
	bool IsPlayerControlled();

	// is my controlled entity
	bool IsControlledEntity();

	// current spec target
	bool IsSpectatedEntity();

	// is controlled or spectated
	bool IsViewedEntity();

	void SetYaw(GFlt yaw, bool updateCamera = true);
	void SetPitch(GFlt pitch, bool updateCamera = true);
	void SetYawDegrees(GFlt yaw, bool updateCamera = true);
	void SetPitchDegrees(GFlt pitch, bool updateCamera = true);
	void SetYawInt(u16 yaw, bool updateCamera = true);
	void SetPitchInt(u16 pitch, bool updateCamera = true);

	u8 Controllable_GetPlayerIndex();

	virtual void OnPossessed() {}
	
	void Controllable_SetInputFromState(u8 state);

};