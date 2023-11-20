#if !_SERVER

#include "interface/render_interface.h"
#include "interface/window.h"
#include "engine/math/glm.h"

KDrawConfig* GetDrawConfig()
{
	return GetRenderInterface()->GetDrawConfig();
}

f32 KDrawConfig::GetVerticalFovRadians()
{
	return glm::radians(VFov);
}

void KDrawConfig::SetVerticalFovRadians(f32 fov)
{
	VFov = glm::degrees(fov);
}

void KDrawConfig::SetVerticalFovDegrees(f32 fov)
{
	VFov = fov;
}

void KDrawConfig::SetHorizontalFovDegrees(f32 fov)
{
	SetVerticalFovDegrees( 2.f * atan( tan(ResX / 2.f) * (ResX / ResY) ) );
}

void KDrawConfig::SetWindowState(EWindowState state)
{
	WindowState = state;
	GetRenderInterface()->GetGameWindow()->SetWindowState(state);
}

#endif