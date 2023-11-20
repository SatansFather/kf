#pragma once

#include "engine/global/types_numeric.h"
#include "engine/math/vec3.h"
#include <mutex>

// the player's view rotation according to their input
class KInputView
{
#if !_SERVER
	friend class KInputProcessor;

	// can be locked from anywhere to prevent mouse input
	static std::atomic<u32> LockedInputCounter;

	static f32 Pitch;
	static f32 Yaw;
	static std::mutex Mutex;

	// for spectator
	static KTimePoint InterStartTime;
	static f64 InterpDuration;
	static f64 InterpRemainingDuration;
	static f32 InterpTargetPitch;
	static f32 InterpTargetYaw;
	static f32 InterpStartPitch;
	static f32 InterpStartYaw;

	static void UpdateFromInput(f64 x, f64 y);

public:
	
	// option to lock the pitch yaw values for threadsafe read
	static void GetViewForRender(f32& p, f32& y);

	static void GetViewForGame(u16& p, u16& y);

	static GFlt GetPitch() { return Pitch; }
	static GFlt GetYaw() { return Yaw; }

	static u16 MapPitchToInt();
	static u16 MapYawToInt();

	// the MapToFloat functions are helpers and are independent from the stored pitch/yaw
	// these functions justify this class existing in the server binary so we can unpack client input
	static f32 MapPitchToFloat(u16 p);
	static f32 MapYawToFloat(u16 y);

	static f32 GetVerticalSensitivity();
	static f32 GetHorizontalSensitivity();

	static void SetAnglesFromVector(const GVec3& dir);
	static void SetPitch(GFlt pitch);
	static void SetYaw(GFlt yaw);
	static void SetPitchDegrees(GFlt pitch);
	static void SetYawDegrees(GFlt yaw);

	static void InterpolatePitchYaw(f32 pitch, f32 yaw, f64 duration);
	static void UpdateInterpolation(f64 delta);

	static void LockInput();
	static void UnlockInput();
#endif

};
