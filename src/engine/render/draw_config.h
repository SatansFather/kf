#pragma once

#if !_SERVER

#include "kfglobal.h"

enum class EDrawMode : u8
{
	Lit,
	Unlit,
	Normal,
	NormalTextured,
	Wireframe
};

enum class EWindowState : u8
{
	Fullscreen,
	Windowed,
	Borderless
};

class KDrawConfig
{
	friend class KUserConfig;

public:

	EDrawMode DrawMode = EDrawMode::Lit;
	f32 ResX = 1600, ResY = 900;
	f32 RenderScale = 1;
	u8 bFilterRenderScale = 0;
	u8 RenderScaleMetric = 1;
	f32 VFov = 70;
	f32 MaxFramerate = 0;
	u8 bInterpolate = 1;
	f32 Brightness = 1;
	f32 Contrast = 1;
	f32 Saturation = 1;
	f32 Flip = 0;
	EWindowState WindowState = EWindowState::Fullscreen;	

	f32 HFov;

public:

	EDrawMode GetDrawMode() { return DrawMode; }
	f32 GetResX() { return ResX; }
	f32 GetResY() { return ResY; }
	f32 GetRenderScale() { return RenderScale; }
	f32 GetVerticalFov() { return VFov; }
	f32 GetVerticalFovRadians();
	EWindowState GetWindowState() { return WindowState; }

	void SetVerticalFovRadians(f32 fov);
	void SetVerticalFovDegrees(f32 fov);
	void SetHorizontalFovDegrees(f32 fov);
	void SetWindowState(EWindowState state);
	void SetResolution(f32 x, f32 y) { ResX = x; ResY = y; }

	bool ScaleRenderByArea() { return RenderScaleMetric != 0; }
};

KDrawConfig* GetDrawConfig();

#endif