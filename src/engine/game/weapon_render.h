#pragma once

#include "kfglobal.h"
#include "engine/math/vec3.h"

// manages the render transform of the carried weapon
class KWeaponRenderInfo
{
	glm::mat4 PrevMat;
	f32 WeaponSwitchAlpha = 0;

	f32 TargetRunSpeedAlpha = 0;
	f32 CurrentRunSpeedAlpha = 0;
	f32 TimeSinceStopped = 0;

	f32 TargetFallSpeed = 0;
	f32 CurrentFallSpeed = 0;

	f32 TargetPitch = 0, CurrentPitch = 0;
	f32 TargetYaw = 0, CurrentYaw = 0;

	f32 LastControlPitch = 0, LastControlYaw = 0;

	f32 TargetRecoil = 0;
	f32 LastTargetRecoilTurnaround = 0;
	f32 CurrentRecoil = 0;
	f32 RecoilRandomSeed = 0;

	f32 InterruptAlpha = -1;

	// relevant for switch interrupts
	GVec3 CurrentPosition;
	GVec3 TargetPosition;
	GVec3 Velocity;
	f32 LastSwitchAlpha = 0;
	f32 CurrentCrouchAlpha = 0;
	f32 TargetCrouchAlpha = 0;
	f32 SwitchDirection = 1;

public:
	
	void Reset();

	glm::mat4 BuildMatrix(const GVec3& currentPos);

	void SetPrevMatrix(const glm::mat4& mat);
	void SetPitch(f32 p);
	void SetYaw(f32 y);
	void SetFallSpeed(f32 speed);
	void SetRunSpeedAlpha(f32 alpha);
	void SetWeaponSwitchAlpha(f32 alpha);
	void SetRecoil(f32 recoil);
	void SetInterruptAlpha(f32 alpha);
	void SetCrouchAlpha(f32 alpha);

	void NegateSwitchDirection() { SwitchDirection *= -1; }
	void ResetSwitchDirection()   { SwitchDirection = 1; }

	const glm::mat4& GetPrevMatrix() const { return PrevMat; }
	f32 GetSwitchAlpha() const { return WeaponSwitchAlpha; }

	// shifting during movement
	GVec3 GetSwing();

	// kick during recoil
	void GetRecoil(GVec3& pos, f32& pitch, f32& yaw, f32& roll);

	// weapon switch position
	bool GetSwitch(GVec3& pos, f32& roll);

	// adjust for crouch position
	void GetCrouch(GVec3& pos);

	// idle sway
	f32 GetIdlePitch();
	f32 GetIdleYaw();

	// look towards turn direction
	f32 GetLookPitch();
	f32 GetLookYaw();
};