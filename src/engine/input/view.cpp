#if !_SERVER

#include "view.h"
#include "engine/math/math.h"
#include "engine/math/glm.h"
#include "engine/game/config.h"
#include "engine/render/interface/render_interface.h"
#include "../game_instance.h"


std::atomic<u32> KInputView::LockedInputCounter = { 0 };
f32 KInputView::Pitch = 0;
f32 KInputView::Yaw = 0;
std::mutex KInputView::Mutex;
KTimePoint KInputView::InterStartTime;
f64 KInputView::InterpDuration;
f64 KInputView::InterpRemainingDuration;
f32 KInputView::InterpStartPitch;
f32 KInputView::InterpStartYaw;
f32 KInputView::InterpTargetPitch;
f32 KInputView::InterpTargetYaw;

void KInputView::UpdateFromInput(f64 x, f64 y)
{
	if (LockedInputCounter > 0 || KGameInstance::Get().bIsMainMenu) return;
	if (GetDrawConfig()->Flip == 1) x = -x;

	Mutex.lock();
	Pitch += -y * glm::radians(GetVerticalSensitivity());
	Yaw += -x * glm::radians(GetHorizontalSensitivity());

	f32 p = glm::radians(89.9f);
	Pitch = std::clamp(Pitch, -p, p);

	Mutex.unlock();
}

void KInputView::GetViewForRender(f32& p, f32& y)
{
	Mutex.lock();
	if (InterpRemainingDuration > 0)
	{
		f64 delta = GetRenderInterface()->GetLastFrameTime();
		InterpRemainingDuration -= delta;
		if (InterpRemainingDuration < 0) InterpRemainingDuration = 0;
		
		f64 alpha = (InterpDuration - InterpRemainingDuration) / InterpDuration;
		Pitch = KLerp(InterpStartPitch, InterpTargetPitch, alpha);

		f32 start = InterpStartYaw;
		f32 target = InterpTargetYaw;

		f32 diff = (target - start);

		if (abs(target + 2.f * PI<f32>() - start) < diff) 
			target += 2.f * PI<f32>();
		else if (abs(target - start + 2.f * PI<f32>()) < diff) 
			start += 2.f * PI<f32>();

		Yaw = KLerp(start, target, alpha);
	}

	p = Pitch;
	y = Yaw;
	Mutex.unlock();
}

void KInputView::GetViewForGame(u16& p, u16& y)
{
	Mutex.lock();
	p = MapPitchToInt();
	y = MapYawToInt();
	Mutex.unlock();
}

u16 KInputView::MapPitchToInt()
{
	GFlt rad = glm::radians(90.f);
	return std::round(MapRange(Pitch, -rad, rad, 0, MAX_I16));
}

u16 KInputView::MapYawToInt()
{
	GFlt rad = glm::radians(180.f);
	return std::round(MapRange(Yaw, -rad, rad, 0, MAX_U16));
}

f32 KInputView::MapPitchToFloat(u16 p)
{
	GFlt rad = glm::radians(90.f);
	if (p == (MAX_I16 / 2) + 1) return 0;
	return MapRange(p, 0, MAX_I16, -rad, rad);
}

f32 KInputView::MapYawToFloat(u16 y)
{
	GFlt rad = glm::radians(180.f);
	if (y == MAX_I16 + 1) return 0;
	return MapRange(y, 0, MAX_U16, -rad, rad);
}

f32 KInputView::GetVerticalSensitivity()
{
	KUserConfig* cfg = GetUserConfig();
	f32 inv = cfg->Mouse.bInvertY ? -1 : 1;
	if (cfg->Mouse.bSeparateMouseAxes)
		return cfg->Mouse.Vertical.GetSensitivity() * inv;

	return cfg->Mouse.Horizontal.GetSensitivity() * inv;
}

f32 KInputView::GetHorizontalSensitivity()
{
	KUserConfig* cfg = GetUserConfig();
	f32 inv = cfg->Mouse.bInvertX ? -1 : 1;
	return cfg->Mouse.Horizontal.GetSensitivity() * inv;
}

void KInputView::SetAnglesFromVector(const GVec3& dir)
{
	GFlt p, y;
	dir.ToType<GFlt>().ToPitchYaw(p, y);
	Pitch = p;
	Yaw = y;
}

void KInputView::SetPitch(GFlt pitch)
{
	Mutex.lock();
	Pitch = pitch;
	Mutex.unlock();
}

void KInputView::SetYaw(GFlt yaw)
{
	Mutex.lock();
	Yaw = yaw;
	Mutex.unlock();
}

void KInputView::SetPitchDegrees(GFlt pitch)
{
	SetPitch(glm::radians(pitch));
}

void KInputView::SetYawDegrees(GFlt yaw)
{
	SetPitch(glm::radians(yaw));
}

void KInputView::InterpolatePitchYaw(f32 pitch, f32 yaw, f64 duration)
{
	Mutex.lock();
	InterpStartPitch = Pitch;
	InterpStartYaw = Yaw;
	InterpTargetPitch = pitch;
	InterpTargetYaw = yaw;
	InterStartTime = KTime::Now();
	InterpDuration = duration;
	InterpRemainingDuration = duration;
	Mutex.unlock();
}

void KInputView::UpdateInterpolation(f64 delta)
{
	Mutex.lock();

	Mutex.unlock();
}

void KInputView::LockInput()
{
	LockedInputCounter++;
}

void KInputView::UnlockInput()
{
	if (LockedInputCounter > 0)
		LockedInputCounter--;
}

#endif