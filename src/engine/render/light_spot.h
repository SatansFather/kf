#pragma once

#if !_SERVER

#include "light.h"
#include "engine/math/vec3.h"

class KLightSpot : public KLight
{
	using KLight::KLight;

public:
	
	glm::vec3 LightDirection;
	glm::vec3 LightPosition;
	f32 OuterAngle = 45;
	f32 MaxRange = 512;

	void SetDirection(const glm::vec3& dir) { LightDirection = dir; }
	void SetDirection(const GVec3& dir) { LightDirection = dir.ToType<f32>().ToGLM(); }

	void SetPosition(const glm::vec3& pos) { LightPosition = pos; }
	void SetPosition(const GVec3& pos) { LightPosition = pos.ToType<f32>().ToGLM(); }

	void BuildLightMatrix() override
	{
		auto view = glm::lookAt(LightPosition, LightPosition + LightDirection, { 0, 1, 0 });
		auto proj = glm::perspectiveFovRH(glm::radians(OuterAngle), f32(DepthRes), f32(DepthRes), 1.f, MaxRange);
		LightMatrix = proj * view;
	}

	/*void __BuildTestMatrix()
	{
		f32 time = KTime::SinceInit() / 1;

		LightDirection = glm::normalize(glm::vec3(sin(time + 1), sin(time * 2) / 5, sin(time)));
		LightDirection = glm::vec3(1, 0, 0);
		//glm::vec3 pos = glm::vec3(0, 256, 0) + LightDirection * 96.f;
		glm::vec3 pos = FVec3(0, 0, 0).ToGLM();

		auto view = glm::lookAt(pos, pos + LightDirection, { 0, 1, 0 });
		auto proj = glm::perspectiveFovRH(glm::radians(55.f), 2048.f, 2048.f, 5.f, 4096.f);

		LightMatrix = proj * view;
	}*/
};

#endif