#include "weapon_render.h"
#include "../utility/random.h"
#include "local_player.h"

void KWeaponRenderInfo::Reset()
{
	memset(this, 0, sizeof(KWeaponRenderInfo));
	PrevMat = glm::mat4(1);
	InterruptAlpha = -1;
	SwitchDirection = 1;
}

glm::mat4 KWeaponRenderInfo::BuildMatrix(const GVec3& currentPos)
{
	glm::mat4 mat = glm::mat4(1);

	GVec3 adjust = GetSwing();

	f32 pitch = GetIdlePitch() - GetLookPitch();
	f32 yaw	= GetIdleYaw() + GetLookYaw();
	f32 roll = 0;

	GetRecoil(adjust, pitch, yaw, roll);

	GetCrouch(adjust);

	bool stateSwitch = GetSwitch(adjust, roll);

#if 0
	if (InterruptAlpha == -1)
			Velocity = stateSwitch ? 0 : Velocity = 
				((adjust + currentPos) - TargetPosition) / GameFrameDelta();

	TargetPosition = adjust + currentPos;

	if (InterruptAlpha != -1/* || (CurrentPosition + adjust + currentPos) != TargetPosition*/)
	{	
		const GVec3 diff = TargetPosition - CurrentPosition;
		GVec3 dir = diff;
		const f32 total = dir.Normalize();
		const f32 move = 50;
		GVec3 travel = dir * KMin(move, total);

		Velocity += travel;
		if (Velocity.Length() > 30) Velocity = Velocity.GetNormalized() * 30;
		CurrentPosition += Velocity * GameFrameDelta();
		if (CurrentPosition.DistanceSq(TargetPosition) < .1)
			CurrentPosition = TargetPosition;
	}
	else
		CurrentPosition = TargetPosition;

#endif
	mat = glm::rotate(mat, pitch, glm::vec3(1, 0, 0));
	mat = glm::translate(mat, (adjust + currentPos).ToGLM());
	mat = glm::rotate(mat, yaw, glm::vec3(0, 1, 0));
	mat = glm::rotate(mat, roll, glm::vec3(0, 0, 1));

	PrevMat = mat; // prev should have already been used outside of this
	return mat;
}

void KWeaponRenderInfo::SetPrevMatrix(const glm::mat4& mat)
{
	PrevMat = mat;
}

void KWeaponRenderInfo::SetPitch(f32 p)
{
	LastControlPitch = TargetPitch;
	TargetPitch = p;
}

void KWeaponRenderInfo::SetYaw(f32 y)
{
	LastControlYaw = TargetYaw;
	TargetYaw = y;
}

void KWeaponRenderInfo::SetFallSpeed(f32 speed)
{
	TargetFallSpeed = -speed / 300;
}

void KWeaponRenderInfo::SetRunSpeedAlpha(f32 alpha)
{
	TargetRunSpeedAlpha = alpha;
}

void KWeaponRenderInfo::SetWeaponSwitchAlpha(f32 alpha)
{
	WeaponSwitchAlpha = alpha;
}

void KWeaponRenderInfo::SetRecoil(f32 recoil)
{
	TargetRecoil = recoil;//KClamp(CurrentRecoil + recoil, 0, 6);
	RecoilRandomSeed = Random();
}

void KWeaponRenderInfo::SetInterruptAlpha(f32 alpha)
{
	if (alpha > .5) alpha -= .5;
	InterruptAlpha = alpha;
}

void KWeaponRenderInfo::SetCrouchAlpha(f32 alpha)
{
	TargetCrouchAlpha = alpha;
}

GVec3 KWeaponRenderInfo::GetSwing()
{
	GVec3 swing;

	f32 alpha = 1;
	if (TimeSinceStopped < .15) alpha = TimeSinceStopped / .15;

	f32 targetRun = TargetFallSpeed != 0 ? 0 : TargetRunSpeedAlpha;

	if (targetRun < CurrentRunSpeedAlpha)
	{
		CurrentRunSpeedAlpha = 
			KClamp(CurrentRunSpeedAlpha - GameFrameDelta() * 5, 
				targetRun, CurrentRunSpeedAlpha);
	}
	else if (targetRun > CurrentRunSpeedAlpha)
	{
		CurrentRunSpeedAlpha = 
			KClamp(CurrentRunSpeedAlpha + GameFrameDelta() * 3,
				CurrentRunSpeedAlpha, targetRun);
	}

	swing.y = sin((KTime::FrameNow()) * 7.5);
	swing.z = sin(KTime::FrameNow() * 15) / 2;
	swing *= CurrentRunSpeedAlpha;

	if (CurrentFallSpeed > TargetFallSpeed)
	{
		CurrentFallSpeed -= GameFrameDelta() * (abs(CurrentFallSpeed - TargetFallSpeed) * .1) * 130;
		if (CurrentFallSpeed < TargetFallSpeed) CurrentFallSpeed = TargetFallSpeed;
	}
	else if (CurrentFallSpeed < TargetFallSpeed)
	{
		CurrentFallSpeed += GameFrameDelta() * (abs(CurrentFallSpeed - TargetFallSpeed) * .1)
			* (CurrentFallSpeed > 0 ? MapRange(CurrentFallSpeed, 0, 1.5, 130, 10) : 130);
		if (CurrentFallSpeed > TargetFallSpeed) CurrentFallSpeed = TargetFallSpeed;
	}
	CurrentFallSpeed = std::clamp(CurrentFallSpeed, -3.f, .9f);
	swing.z += CurrentFallSpeed;

	return swing;
}

void KWeaponRenderInfo::GetRecoil(GVec3& pos, f32& pitch, f32& yaw, f32& roll)
{
	f32 alpha = 0;
	f32 target = 0;
	if (CurrentRecoil < TargetRecoil)
	{
		CurrentRecoil = KClamp(CurrentRecoil + GameFrameDelta() * 40, KMin(CurrentRecoil, TargetRecoil), TargetRecoil);
		alpha = CurrentRecoil / TargetRecoil;
		alpha = sin(PI<GFlt>() * alpha / 2.0);
		target = TargetRecoil;

	}
	else if (CurrentRecoil > 0 && TargetRecoil == 0)
	{
		CurrentRecoil = KClamp(CurrentRecoil - GameFrameDelta() * 13, TargetRecoil, CurrentRecoil);
		alpha = CurrentRecoil / LastTargetRecoilTurnaround;
		alpha = LerpFade(alpha);
		target = LastTargetRecoilTurnaround;
	}

	if (CurrentRecoil == TargetRecoil)
	{
		LastTargetRecoilTurnaround = TargetRecoil;
		TargetRecoil = 0;
	}

	pitch -= alpha * target * .015;
	roll -= alpha * target * .035 * MapRange(RecoilRandomSeed, 0, 255, -1, 1);
	pos += GVec3(-target * 1.5, 0, target / 2) * alpha;
}

bool KWeaponRenderInfo::GetSwitch(GVec3& pos, f32& roll)
{
	f32 switchAlpha = pow(1 - abs(WeaponSwitchAlpha - .5) * 2, 2.5);
	pos.z -= switchAlpha * 12;


	f32 switchScale = WeaponSwitchAlpha < .5 ? 1 : -1;
	switchScale *= SwitchDirection;

	roll += (pow(switchAlpha, .8) / 1) * 1.3 * switchScale;
	pos.y -= (pow(switchAlpha, .8) / 1) * 13.5 * switchScale;

	bool stateSwitch = (WeaponSwitchAlpha >= .5) != (LastSwitchAlpha >= .5);
	LastSwitchAlpha = WeaponSwitchAlpha;

	return stateSwitch;
}

void KWeaponRenderInfo::GetCrouch(GVec3& pos)
{
	if (CurrentCrouchAlpha != TargetCrouchAlpha)
	{
		f32 diff = TargetCrouchAlpha - CurrentCrouchAlpha;
		CurrentCrouchAlpha += diff / 8;
		CurrentCrouchAlpha = KSaturate(CurrentCrouchAlpha);
	}
	pos.z -= 1.5 * CurrentCrouchAlpha;
}

f32 KWeaponRenderInfo::GetIdlePitch()
{
	return glm::radians(KLerp(
		sin(KTime::FrameNow() * 2),
		0, CurrentFallSpeed != 0 ? 0 : CurrentRunSpeedAlpha));
}

f32 KWeaponRenderInfo::GetIdleYaw()
{
	return glm::radians(KLerp(
		sin(KTime::FrameNow() * 1.5) * .5,
		0, CurrentRunSpeedAlpha));
}

f32 KWeaponRenderInfo::GetLookPitch()
{

	if (GetLocalPlayer()->bTeleportedThisFrame)
	{
		CurrentPitch = TargetPitch;
		LastControlPitch = CurrentPitch;
	}
	else
	{
		f32 pitchRate = (TargetPitch - LastControlPitch) / GameFrameDelta() / 66;
		if (CurrentPitch > pitchRate)
		{
			CurrentPitch -= GameFrameDelta() * (abs(pitchRate - CurrentPitch) * .1) * 50;
			if (CurrentPitch < pitchRate) CurrentPitch = pitchRate;
		}
		else if (CurrentPitch < pitchRate)
		{
			CurrentPitch += GameFrameDelta() * (abs(pitchRate - CurrentPitch) * .1) * 50;
			if (CurrentPitch > pitchRate) CurrentPitch = pitchRate;
		}
	}

	return CurrentPitch;
}

f32 KWeaponRenderInfo::GetLookYaw()
{
	if (GetLocalPlayer()->bTeleportedThisFrame)
	{
		CurrentYaw = TargetYaw;
		LastControlYaw = CurrentYaw;
	}
	else
	{

		f32 last = LastControlYaw;
		f32 target = TargetYaw;

		// prevent overflow wraparound from skewing result
		const f32 maxRad = PI<f32>() * 2;
		if (abs(last - maxRad - target) < last - target)
			last -= maxRad;
		else if (abs(target - maxRad - last) < target - last)
			target -= maxRad;

		f32 yawRate = (target - last) / GameFrameDelta() / 35;
		yawRate = KClamp(yawRate, -.18, .18);

		if (CurrentYaw > yawRate)
		{
			CurrentYaw -= GameFrameDelta() * (abs(yawRate - CurrentYaw) * .1) * 100;
			if (CurrentYaw < yawRate) CurrentYaw = yawRate;
		}
		else if (CurrentYaw < yawRate)
		{
			CurrentYaw += GameFrameDelta() * (abs(yawRate - CurrentYaw) * .1) * 75;
			if (CurrentYaw > yawRate) CurrentYaw = yawRate;
		}
	}

	return CurrentYaw;
}
