#if !_SERVER

#include "camera_push.h"

void KCameraPush::PushRotation(const FVec3& amount, f32 duration, f32 startAlpha /*= 0*/)
{
	if (amount.LengthSq() > Rotation.Rotation.LengthSq())
	{
		Rotation.Rotation = amount;
		Rotation.Duration = duration;
		Rotation.Alpha = startAlpha;
		Flags |= HasRotation;
	}
}

void KCameraPush::PushFov(f32 amount, f32 duration, f32 startAlpha /*= 0*/)
{
	if (amount > Fov.Fov)
	{
		Fov.Fov = amount * PI<f32>() / 180.f;
		Fov.Duration = duration;
		Fov.Alpha = startAlpha;
		Flags |= HasFov;
	}
}

void KCameraPush::PushShake(f32 strength, f32 duration, f32 shakeRate)
{
	if (strength > Shake.Strength)
	{
		Shake.Strength = strength;
		Shake.Duration = duration;
		Shake.Rate = shakeRate;
		Flags |= HasShake;
	}
}

void KCameraPush::AddConcussion(f32 value)
{
	Concussion += value;
	Flags |= HasConcussion;
}

void KCameraPush::UpdateForRender(KCameraPush& renderPush)
{
	if (Flags & HasShake)
	{
		renderPush.Shake = Shake;
		renderPush.Shake.Time = KTime::Now();
	}

	if (Flags & HasFov)
	{
		renderPush.Fov = Fov;
		renderPush.Fov.Time = KTime::Now();
	}

	if (Flags & HasRotation)
	{
		renderPush.Rotation = Rotation;
		renderPush.Rotation.Time = KTime::Now();
	}

	if (Flags & HasConcussion)
		renderPush.Concussion += Concussion;

	renderPush.Flags = Flags;
}

void KCameraPush::Reset()
{
	memset(this, 0, sizeof(KCameraPush));
}

KCameraPush KCameraPush::operator*(const f32 scale)
{
	KCameraPush push = *this;
	push.Rotation.Rotation *= scale;
	push.Fov.Fov *= scale;
	push.Shake.Strength *= scale;
	return push;
}

#endif