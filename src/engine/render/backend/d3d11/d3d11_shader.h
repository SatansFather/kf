#pragma once

#if !_SERVER && _WIN32

#include "engine/render/interface/shader.h"
#include "d3d11_include.h"
#include "engine/utility/k_assert.h"

// ugly macro but prevents all shaders from copy pasting this
#define CREATE_SHADER(type)						\
												\
K_ASSERT_HR(GetDevice()->Create##type##Shader(	\
	blob->GetBufferPointer(),					\
	blob->GetBufferSize(),						\
	NULL,										\
	&Shader), "could not create shader");

class KD3D11Shader
{
public:
	void* ShaderCode;
	u32 CodeSize;
	virtual void InitFromBlob(ID3DBlob* blob) = 0;
	virtual void InitFromPreSetData() = 0;
};

class KShaderVertex_D3D11 : public KShaderVertex, public KD3D11Shader, public D3D11Object
{
	friend class KShaderProgram_D3D11;

private:

	ComPtr<ID3D11VertexShader> Shader;
	ComPtr<ID3D11InputLayout> InputLayout;


public:
	
	void InitFromBlob(ID3DBlob* blob) override
	{	
		ShaderCode = blob->GetBufferPointer();
		CodeSize = blob->GetBufferSize();

		CREATE_SHADER(Vertex);
	}

	void InitFromPreSetData() override
	{
		GetDevice()->CreateVertexShader(ShaderCode, CodeSize, nullptr, &Shader);
	}

	void SetInputLayout(const struct FInputLayout& layout) override;

	/*void SetInputLayout(TVector<D3D11_INPUT_ELEMENT_DESC> inputlayout, void* shader_code, SIZE_T code_size)
	{
		GetDevice()->CreateInputLayout(inputlayout.data(), u8(inputlayout.size()), shader_code, code_size, &InputLayout);
	}*/
};

class KShaderPixel_D3D11 : public KShaderPixel, public KD3D11Shader, public D3D11Object
{
	friend class KShaderProgram_D3D11;

private:

	ComPtr<ID3D11PixelShader> Shader;

public:

	void InitFromBlob(ID3DBlob* blob) override
	{
		CREATE_SHADER(Pixel);
	}

	void InitFromPreSetData() override
	{
		GetDevice()->CreatePixelShader(ShaderCode, CodeSize, nullptr, &Shader);
	}
};

class KShaderCompute_D3D11 : public KShaderCompute, public KD3D11Shader, public D3D11Object
{
	friend class KShaderProgram_D3D11;

private:

	ComPtr<ID3D11ComputeShader> Shader;

public:

	void InitFromBlob(ID3DBlob* blob) override
	{
		CREATE_SHADER(Compute);
	}

	void InitFromPreSetData() override
	{
		GetDevice()->CreateComputeShader(ShaderCode, CodeSize, nullptr, &Shader);
	}
};


#endif