#if !_SERVER && _WIN32

#include "d3d11_shader_program.h"
#include "comdef.h"
#include "d3d11_include.h"
#include "engine/utility/k_assert.h"
#include "d3d11_shader.h"
#include "engine/global/paths.h"
#include <d3dcompiler.h>

void KShaderProgram_D3D11::CompileFromFile(EShaderStage stage, const KString& name, KShader* target)
{
#if _PACK
	return;
#endif
	// determine shader type + level
	KString profile;
	switch (stage)
	{
		case EShaderStage::Vertex:
		{
			profile = "vs_";
			break;
		}
		case EShaderStage::Pixel:
		{
			profile = "ps_";
			break;
		}
		case EShaderStage::Compute:
		{
			profile = "cs_";
			break;
		}
		default:
		{
			K_ASSERT(false, "could not compile shader of invalid type");
			return;
		}
	}
	// use SM5
	profile += "5_0";

	u32 flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* blob = nullptr;
	ID3DBlob* err = nullptr;

	// create path from name
	KString path = HLSL_DIR + name + ".hlsl";

	// dont wrap this in CheckHR because we need the error compiler errors
	HRESULT hr = D3DCompileFromFile(
		path.ToWideStr().c_str(),
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		profile,
		flags,
		0,
		&blob,
		&err
	);

	// provide error messages
	if (FAILED(hr))
	{
		_com_error com(hr);
		LPCTSTR hr_msg = com.ErrorMessage();

		if (blob) blob->Release();

		char* error = nullptr;
		if (err)
		{
			error = (char*)err->GetBufferPointer();
			K_ASSERT_HR(hr, KString("failed to compile shader ") + path + KString("\n\n") + KString(error));
			err->Release();
		}
		else
		{
			K_ASSERT_HR(hr, "failed find shader\n \"" + path + "\"\n");
		}

		return;
	}

	switch (stage)
	{
		case EShaderStage::Vertex:
		{
			KShaderVertex_D3D11* shader = dynamic_cast<KShaderVertex_D3D11*>(target);
			K_ASSERT(shader, "shader class type did not match EShaderStage");
			shader->InitFromBlob(blob);

			// do this now while the blob pointer is still valid
			shader->SetInputLayout(CreationParams.InputLayout);

			break;
		}
		case EShaderStage::Pixel:
		{
			KShaderPixel_D3D11* shader = dynamic_cast<KShaderPixel_D3D11*>(target);
			K_ASSERT(shader, "shader class type did not match EShaderStage");
			shader->InitFromBlob(blob);
			break;
		}
		case EShaderStage::Compute:
		{
			KShaderCompute_D3D11* shader = dynamic_cast<KShaderCompute_D3D11*>(target);
			K_ASSERT(shader, "shader class type did not match EShaderStage");
			shader->InitFromBlob(blob);
			break;
		}
	}

	blob->Release();
}

void KShaderProgram_D3D11::CompileFromString(EShaderStage stage, const KString& code, KShader* target)
{

}

void KShaderProgram_D3D11::FromPrecompiled(EShaderStage stage, void* data, size_t size, KShader* target)
{
	switch(stage)
	{
		case EShaderStage::Vertex:
		{
			KShaderVertex_D3D11* shader = dynamic_cast<KShaderVertex_D3D11*>(target);
			shader->CodeSize = size;
			shader->ShaderCode = data;
			shader->SetInputLayout(CreationParams.InputLayout);
			shader->InitFromPreSetData();
			break;
		}
		case EShaderStage::Pixel:
		{
			KShaderPixel_D3D11* shader = dynamic_cast<KShaderPixel_D3D11*>(target);
			shader->CodeSize = size;
			shader->ShaderCode = data;
			shader->InitFromPreSetData();
			break;
		}
	}
}

UPtr<KShaderVertex> KShaderProgram_D3D11::CreateVertexShader()
{
	return std::make_unique<KShaderVertex_D3D11>();
}

UPtr<KShaderPixel> KShaderProgram_D3D11::CreatePixelShader()
{
	return std::make_unique<KShaderPixel_D3D11>();;
}

UPtr<KShaderCompute> KShaderProgram_D3D11::CreateComputeShader()
{
	return std::make_unique<KShaderCompute_D3D11>();;
}

void KShaderProgram_D3D11::BindVertexShader(class KShaderVertex* shader)
{
	// temporary cast to d3d11 shader until generic input layout is made
	KShaderVertex_D3D11* vert = dynamic_cast<KShaderVertex_D3D11*>(shader);
	
	auto context = GetContext();
	context->VSSetShader(vert->Shader.Get(), nullptr, 0);
	context->IASetInputLayout(vert->InputLayout.Get());
}

void KShaderProgram_D3D11::BindPixelShader(class KShaderPixel* shader)
{
	KShaderPixel_D3D11* vert = dynamic_cast<KShaderPixel_D3D11*>(shader);
	GetContext()->PSSetShader(vert->Shader.Get(), nullptr, 0);
}

void KShaderProgram_D3D11::BindComputeShader(class KShaderCompute* shader)
{
	KShaderCompute_D3D11* comp = dynamic_cast<KShaderCompute_D3D11*>(shader);
	GetContext()->CSSetShader(comp->Shader.Get(), nullptr, 0);
}

void KShaderProgram_D3D11::BindProgram()
{
	if (VertexShader) BindVertexShader(VertexShader.get());
	if (PixelShader) BindPixelShader(PixelShader.get());
	if (ComputeShader) BindComputeShader(ComputeShader.get());
}

#endif