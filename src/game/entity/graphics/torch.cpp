#if !_SERVER

#include "torch.h"

KBufferUpdateResult KEntity_WallTorch::UpdateBuffers(KStaticMesh<"torch1", "kf/metal2">& mesh, KTorchFlame& flame)
{
	glm::mat4 mat(1);

	mat = glm::translate(mat, GetPosition().ToGLM());
	mat = glm::scale(mat, glm::vec3(2, 2, 2));
	mat = glm::rotate(mat, glm::radians(Yaw), glm::vec3(0, 1, 0));

	mesh.PrevModelMat = mat;
	mesh.CurrentModelMat = mat;
	mesh.SetLastMoveRenderAlpha(1);

	MainColor.SetA(200);
	OffColor.SetA(100);
	flame.SetPosition(GetPosition() + (GVec3(4.6, 0, 10) * 2).GetRotatedDegrees(Yaw, GVec3(0, 0, 1)));
	flame.SetMainColor(MainColor);
	flame.SetOffColor(OffColor);

	return KBufferUpdateResult(false, true);
}

#endif