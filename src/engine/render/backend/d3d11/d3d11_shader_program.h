#pragma once

#if !_SERVER && _WIN32

#include "engine/render/interface/shader_program.h"
#include "d3d11_include.h"

class KShaderProgram_D3D11 : public KShaderProgram, public D3D11Object
{
public:
	void CompileFromFile(EShaderStage stage, const KString& name, KShader* target) override;
	void CompileFromString(EShaderStage stage, const KString& code, KShader* target) override;
	void FromPrecompiled(EShaderStage stage, void* data, size_t size, KShader* target) override;
	UPtr<KShaderVertex> CreateVertexShader() override;
	UPtr<KShaderPixel> CreatePixelShader() override;
	UPtr<KShaderCompute> CreateComputeShader() override;
	void BindVertexShader(class KShaderVertex* shader);
	void BindPixelShader(class KShaderPixel* shader);
	void BindComputeShader(class KShaderCompute* shader);
	void BindProgram() override;
};

#endif