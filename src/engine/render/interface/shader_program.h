#pragma once

#if !_SERVER

#include "shader.h"
#include "engine/render/shader_stage.h"
#include "../input_layout.h"

enum class EShaderLoadMethod
{
	Invalid,
	Precompiled,
	FromFile,
	FromString
};

struct FShaderProgramCompileInfo
{
	EShaderLoadMethod LoadMethod = EShaderLoadMethod::Invalid;

	// if method is FromFile
	KString FileName; 

	// if method is FromString
	KString CodeString;	

	// if method is Precompiled
	void* Data;
	size_t Size;
};

struct FShaderProgramCreateParams
{
	FShaderProgramCompileInfo VertexShader;
	FInputLayout InputLayout;

	FShaderProgramCompileInfo PixelShader;

	FShaderProgramCompileInfo ComputeShader;
};

class KShaderProgram
{
	friend class KRenderInterface;

//protected:
public:

	UPtr<KShaderVertex> VertexShader;
	UPtr<KShaderPixel> PixelShader;
	UPtr<KShaderCompute> ComputeShader;

	FShaderProgramCreateParams CreationParams;

protected:

	// pass the filename without path or extension
	virtual void CompileFromFile(EShaderStage stage, const KString& name, KShader* target) = 0;

	// compiles from an existing string
	virtual void CompileFromString(EShaderStage stage, const KString& code, KShader* target) = 0;

	// uses a precompiled byte buffer to create a shader
	virtual void FromPrecompiled(EShaderStage stage, void* data, size_t size, KShader* target) = 0;

	// creates an empty vertex shader
	virtual UPtr<KShaderVertex> CreateVertexShader() = 0;

	// creates an empty pixel shader
	virtual UPtr<KShaderPixel> CreatePixelShader() = 0;

	// creates an empty compute shader
	virtual UPtr<KShaderCompute> CreateComputeShader() = 0;

	virtual void BindProgram() = 0;

	// not abstract because only GL needs this
	virtual void FinalizeProgram() {}

public:

	virtual ~KShaderProgram() = default;

	/* OpenGL derived class needs to create/destroy the program in its constructor/destructor
	*  D3D11 does not share the concept of a "program" and does not need to do this */

	void CreateShadersFromParams(const FShaderProgramCreateParams& params);
	
private:

	// do not call this
	// call BindShaderProgram from interface
};

#endif