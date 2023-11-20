#if !_SERVER

#include "camera.h"
#include "interface/buffers/gpu_buffer.h"
#include "engine/input/view.h"
#include "communication.h"
#include "draw_config.h"

// DELETE
#include "../system/time.h"
#include "../input/broadcast.h"
#include "../system/terminal/terminal.h"
#include "../math/vec3.h"
#include "../game_instance.h"
#include "../../game/testmover.h"
#include "interface/render_interface.h"
#include "../collision/trace.h"

KRenderCamera::~KRenderCamera() {}

void KRenderCamera::SetRadiosityPerspective(const glm::vec3& point, const glm::vec3& norm, const glm::vec3& up, f32 res)
{
	KRenderInterface* iface = GetRenderInterface();

	ViewMatrix = glm::lookAtRH(point, point + norm, up);
	//glm::mat4 view = glm::lookAtRH(point, point + norm, { 0.f, 1.f, 0.f });
	ProjectionMatrix = glm::perspectiveFovRH(
		glm::radians(90.f),
		//glm::radians(179.f),
		res, res,
		5.f, 100000.f);

	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	iface->UpdateBuffer(iface->ViewBuffer.get(), &ViewMatrix);

	CameraData.Position = glm::vec4(point, 1);
	CameraData.Direction = glm::vec4(norm, 1);
	iface->UpdateBuffer(iface->CameraDataBuffer.get(), &CameraData);
}

void KRenderCamera::UpdateFromRenderBridge()
{
#if !_COMPILER
	LastRoll = KRenderBridge::Get().GetLastCameraRoll();
	CurrentRoll = KRenderBridge::Get().GetCameraRoll();
	LastPos = KRenderBridge::Get().GetLastCameraPosition();
	CurrentPos = KRenderBridge::Get().GetCameraPosition();
#endif
}

void KRenderCamera::SetFromPlayer()
{
#if !_COMPILER
	KRenderInterface* iface = GetRenderInterface();

	glm::vec3 pos = glm::lerp(
		LastPos,
		CurrentPos,
		iface->GetRenderAlpha());

	FColor32 waterColor;
	PointIsInWater(GVec3::FromGLM(pos), waterColor);
	CameraData.UnderWaterColor = glm::vec4(waterColor.r, waterColor.g, waterColor.b, waterColor.a);
	
	SetFromPosition(pos);
#endif
}

void KRenderCamera::SetFromPosition(const glm::vec3& pos)
{
#if !_COMPILER
	f32 rotP = 0; f32 rotY = 0;
	KInputView::GetViewForRender(rotP, rotY);

	CalculatePushForFrame();

	rotP += AddRotationThisFrame.x + ShakeRotationThisFrame.x;
	rotY += AddRotationThisFrame.y + ShakeRotationThisFrame.y;

	const f32 rad90 = PI<f32>() / 2.f - .001;
	rotP = std::clamp(rotP, -rad90, rad90);

	glm::vec3 dir
	(
		cos(rotP) * sin(rotY),
		sin(rotP),
		cos(rotP) * cos(rotY)
	);

	// used to flip rendering when over 90 degrees
	LastRecordedPitch = rotP;

	SetFromPositionAndDirection(pos, dir);
#endif
}

void KRenderCamera::SetFromPositionAndDirection(const glm::vec3& pos, const glm::vec3& dir)
{
#if !_COMPILER
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	KDrawConfig* cfg = GetDrawConfig();

	LockConfig();
	f32 fov = cfg->GetVerticalFovRadians();
	fov += AddFovThisFrame;
	UnlockConfig();

	f32 roll = glm::lerp(LastRoll, CurrentRoll, iface->GetRenderAlpha());
	roll += AddRotationThisFrame.z + ShakeRotationThisFrame.z;
	glm::vec3 up = glm::rotate(glm::vec3(0, 1, 0), roll, dir);

	glm::vec3 position = pos + ShakePositionThisFrame;

	ViewMatrix = glm::lookAtRH(pos, pos + dir, up);
	ProjectionMatrix = glm::perspectiveFovRH(
		fov,
		GetRenderResX(),
		GetRenderResY(),
		5.f, 100000.f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	struct { glm::mat4 v, p, sv; } views;
	views.v = ViewMatrix;
	views.p = ProjectionMatrix;


	glm::vec3 dir2 = dir;
	dir2.y = 0;
	dir2 = glm::normalize(dir2);
	views.sv = glm::lookAtRH(pos, pos + dir2, { 0, 1, 0 } );

	iface->UpdateBuffer(iface->ViewBuffer.get(), &views);

	CameraData.Position = glm::vec4(pos, 1);
	CameraData.Direction = glm::vec4(dir, 1);
	CameraData.Concussion = Push.Concussion;
	// TODO put cam pitch here instead of frame data

	iface->UpdateBuffer(iface->CameraDataBuffer.get(), &CameraData);
#endif
}

void KRenderCamera::SetForPlayerWeapon()
{
#if !_COMPILER
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	constexpr f32 fov = glm::radians(65.f);
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 pos(0, 0, 0);
	glm::vec3 dir(0, 0, 1);

	glm::mat4 view = glm::lookAtRH(pos, pos + dir, up);
	glm::mat4 proj = glm::perspectiveFovRH(
		fov,
		GetRenderResX(),
		GetRenderResY(),
		.01f, 100.f);

	ViewProjectionMatrix = proj * view;

	struct { glm::mat4 v, p; } views;
	views.v = view;
	views.p = proj;
	iface->UpdateBuffer(iface->ViewBuffer.get(), &views);

	CameraData.Position = glm::vec4(pos, 1);
	CameraData.Direction = glm::vec4(dir, 1);
	CameraData.Concussion = Push.Concussion;

	iface->UpdateBuffer(iface->CameraDataBuffer.get(), &CameraData);
#endif
}

void KRenderCamera::CalculatePushForFrame()
{
	AddFovThisFrame = 0;
	AddRotationThisFrame = 0;
	ShakeRotationThisFrame = 0;
	ShakePositionThisFrame = glm::vec3(0, 0, 0);

	KRenderInterface* iface = GetRenderInterface();
	f32 dilation = iface->GetTimeDilation();

	if (Push.Rotation.Rotation != 0)
	{
		Push.Rotation.Alpha = KTime::Since(Push.Rotation.Time) * dilation / Push.Rotation.Duration;

		if (Push.Rotation.Alpha >= 1)
		{
			Push.Rotation.Alpha = 0;
			Push.Rotation.Rotation = 0;
			Push.Rotation.Duration = 0;
		}
		else
		{
			f32 x = Push.Rotation.Alpha;
			f32 y = (sin((2.f * x * PI<f32>()) - (PI<f32>() / 2.f)) / 2.f) + .5;
			AddRotationThisFrame = Push.Rotation.Rotation * glm::radians(y);
		}
	}

	if (Push.Fov.Fov > 0)
	{
		Push.Fov.Alpha = KTime::Since(Push.Fov.Time) * dilation / Push.Fov.Duration;

		if (Push.Fov.Alpha >= 1)
		{
			Push.Fov.Alpha = 0;
			Push.Fov.Fov = 0;
			Push.Fov.Duration = -1;
		}
		else
		{
			f32 x = Push.Fov.Alpha;
			f32 y = (sin((2.f * x * PI<f32>()) - (PI<f32>() / 2.f)) / 2.f) + .5;
			AddFovThisFrame = Push.Fov.Fov * y;
		}
	}

	f32 shakeTime = iface->GetTotalRenderTime() * dilation;
	f32 shakeStrength = Push.Shake.Strength * (1 - (KTime::Since(Push.Shake.Time) * dilation) / Push.Shake.Duration);
	f32 radStrength = glm::radians(shakeStrength);
	if (shakeStrength > 0)
	{
		ShakeRotationThisFrame.x += RandomSmoothValue(shakeTime * Push.Shake.Rate + 3, radStrength);
		ShakeRotationThisFrame.y += RandomSmoothValue(shakeTime * Push.Shake.Rate, radStrength);
		ShakeRotationThisFrame.z += RandomSmoothValue(shakeTime * Push.Shake.Rate + 5, radStrength);

		ShakePositionThisFrame.x += RandomSmoothValue(shakeTime * Push.Shake.Rate + 3, shakeStrength);
		ShakePositionThisFrame.y += RandomSmoothValue(shakeTime * Push.Shake.Rate, shakeStrength);
		ShakePositionThisFrame.z += RandomSmoothValue(shakeTime * Push.Shake.Rate + 5, shakeStrength);
	}

	if (Push.Concussion > 0)
	{
		const f32 rate = .5;
		const f32 baseDelta = .05;
		const f32 delta = iface->GetLastFrameTime() * dilation;
		const f32 ratio = delta / baseDelta;

		if (delta > baseDelta)
		{
			f32 amount = rate / ratio;
			Push.Concussion *= amount;
		}
		else
		{
			Push.Concussion *= 1 - ratio * rate;
		}

		if (Push.Concussion < .0005)
			Push.Concussion = 0;
	}
}

#endif
