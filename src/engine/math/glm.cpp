#include "glm.h"
#include "glm/ext/matrix_transform.hpp"

void BuildModelMatrix(glm::mat4& out, glm::vec3 pos, AxisAngle rot, glm::vec3 scale)
{
	out = glm::translate(out, pos);
	out = glm::scale(out, scale);
	out = glm::rotate(out, glm::radians(rot.Angle), rot.Axis);


}
