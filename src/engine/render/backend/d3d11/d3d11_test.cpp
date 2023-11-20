#if !_SERVER && _WIN32 && 0

#include "buffers/d3d11_gpu_buffer.h"
#include "d3d11_interface.h"
#include "d3d11_include.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_AVX
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/detail/qualifier.hpp"
#include "../../../system/terminal/terminal.h"
#include "glm/ext/matrix_transform.hpp"

f64 total = 0;
u32 count = 0;

#if 0

XMMATRIX Model;
XMMATRIX View;
XMMATRIX Projection;
XMMATRIX RotationMatrix;

XMVECTOR Position = XMVectorSet(0.f, 0.0f, -0.f, 0.0f);;
XMVECTOR Target;

const XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
const XMVECTOR DefaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

struct TransformData
{
	XMMATRIX WVP;
};

TransformData Transform;

void KRenderInterface_D3D11::__TestCameraInit()
{
	Model = XMMatrixIdentity();


	// set projection
}

void KRenderInterface_D3D11::__TestCameraUpdate()
{

	KConstantBuffer_D3D11<TransformData> TransformConstantBuffer;
	TransformConstantBuffer.CreateStatic(0, EShaderStage::Vertex);
	ImmediateContext->VSSetConstantBuffers(0, 1, TransformConstantBuffer.Buffer.GetAddressOf());
	
	KSystemTerminal::ShowTerminal();
	FTimePoint start = KTime::Now();
	// calculate pitch yaw rotation
	//RotationMatrix = XMMatrixRotationRollPitchYaw(0, KTime::SinceInit() * 30 * (3.14159 / 180.0), 0);
	//XMVECTOR target = XMVector3TransformCoord(DefaultForward, RotationMatrix);
	//target = XMVector3Normalize(target);

	Projection = XMMatrixPerspectiveFovLH(56 * (3.14159 / 180.0), 1600.f / 900.f, .1, 50);
	// calculate up direction based on roll
	XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	// update position
	XMFLOAT4 pos(0, 0, -10, 1);

	Position = XMLoadFloat4(&pos);

	XMVECTOR target = XMVectorSet(0, 0, 1, 0);

	Target = Position + target;

	// set final view matrix
	View = XMMatrixLookAtLH(Position, Target, up);

	// update constant buffer
	Transform.WVP = Model * View * Projection;
	Transform.WVP = XMMatrixTranspose(Transform.WVP);

	total += KTime::Since(start);
	count ++;
	SYSLOG((total / (f64)count));


	TransformConstantBuffer.Update(&Transform);
}

// #else

struct TransformData
{
	glm::mat4 MVP;
};

TransformData Transform;

void KRenderInterface_D3D11::__TestCameraInit()
{

}

void KRenderInterface_D3D11::__TestCameraUpdate()
{
	auto TransformConstantBuffer = CreateConstantBufferStatic(sizeof(TransformData), 0, EShaderStage::Vertex);
	BindConstantBuffer(TransformConstantBuffer.get());

	glm::vec3 position(0, 0, -10);
	glm::vec4 dir(0, 0, 1, 1);
	
	//glm::vec4 target(0, 0, 1, 1);
	//auto rot = glm::eulerAngleYZX(f32(KTime::SinceInit()) * 30.f * (3.14159f / 180.f), 0.f, 0.f);
	//target = target * rot;
	//
	//glm::vec3 tar(target.x, target.y, target.z);
	//tar = glm::normalize(tar);
	
	//auto rotQ = glm::angleAxis(glm::radians(f32(KTime::SinceInit()) * 45.f), glm::vec3(0, 1, 0));
	//auto rot = glm::mat4_cast(rotQ);
	//
	//dir = dir * rot;

	auto gl_m = glm::mat4(1.0f);
	gl_m = glm::translate(gl_m, glm::vec3(0, KTime::SinceInit(), 0));

	auto gl_v = glm::lookAtLH(position, position + glm::vec3(dir.x, dir.y, dir.z), { 0, 1, 0 });
	auto gl_p = glm::perspectiveFovLH(glm::radians(56.f), 1600.0f, 900.f, 0.1f, 10000.0f);

	glm::mat4 gl_pv = gl_p * gl_v;
	
	Transform.MVP = glm::transpose(gl_pv * gl_m);

	// update constant buffer
	UpdateConstantBuffer(TransformConstantBuffer.get(), &Transform);
}

#endif

#endif
