#pragma once

//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#define GLM_FORCE_ALIGNED
//#define GLM_FORCE_AVX
//#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/euler_angles.hpp"

#include "engine/global/types_numeric.h"

// axis-angle rotation
struct AxisAngle
{
	glm::vec3 Axis;

	// degrees (for humans)
	f32 Angle = 0;
};

struct Euler
{
	f32 pitch = 0;
	f32 yaw = 0;
	f32 roll = 0;
};

void BuildModelMatrix(glm::mat4& out, glm::vec3 pos, AxisAngle rot, glm::vec3 scale);

