#pragma once

#if !_SERVER

#include "engine/math/glm.h"
#include "camera_push.h"
#include <memory>

class KRenderCamera
{
	friend class KRenderInterface;

private:

	f32 CurrentRoll, LastRoll;
	f32 LastRecordedPitch;
	glm::vec3 CurrentPos, LastPos;
	glm::mat4 ViewProjectionMatrix;
	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	KCameraPush Push;
	FVec3 AddRotationThisFrame;
	f32 AddFovThisFrame = 0;
	FVec3 ShakeRotationThisFrame;
	glm::vec3 ShakePositionThisFrame;

	// fed to the constant buffer
	struct 
	{
		glm::vec4 Position;
		glm::vec4 Direction;
		f32 Concussion = 0;
		glm::vec3 pad;
		glm::vec4 UnderWaterColor;
		
	} CameraData;

	void CalculatePushForFrame();

public:

	// updates ViewProjectionMatrix according to the player's camera
	// also updates the CameraData constant buffer on the GPU
	void SetFromPlayer();

	void SetFromPosition(const glm::vec3& pos);
	void SetFromPositionAndDirection(const glm::vec3& pos, const glm::vec3& dir);

	void SetForPlayerWeapon();

	virtual ~KRenderCamera();

	void SetRadiosityPerspective(const glm::vec3& point, const glm::vec3& norm, const glm::vec3& up, f32 res);

	inline const glm::mat4& GetViewProjectionMatrix() const { return ViewProjectionMatrix; }

	glm::vec4 GetPosition() { return CameraData.Position; }
	glm::vec4 GetDirection() { return CameraData.Direction; }

	// assumes bridge is locked
	void UpdateFromRenderBridge();
};

#endif