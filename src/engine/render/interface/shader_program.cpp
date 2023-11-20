#if !_SERVER

#include "shader_program.h"

#if _PACK
#include <fstream>
#endif

#define ASSIGN_SHADER(type)																						\
if (params.##type##Shader.LoadMethod != EShaderLoadMethod::Invalid)												\
{																												\
	##type##Shader = Create##type##Shader();																	\
																												\
	switch (params.##type##Shader.LoadMethod)																	\
	{																											\
		case EShaderLoadMethod::FromFile:																		\
		{																										\
			CompileFromFile(EShaderStage::##type##, params.##type##Shader.FileName, ##type##Shader.get());		\
			break;																								\
		}																										\
		case EShaderLoadMethod::FromString:																		\
		{																										\
			CompileFromString(EShaderStage::##type##, params.##type##Shader.CodeString, ##type##Shader.get());	\
			break;																								\
		}																										\
		case EShaderLoadMethod::Precompiled:																	\
		{																										\
			FromPrecompiled(EShaderStage::##type##, params.##type##Shader.Data, params.##type##Shader.Size, ##type##Shader.get());\
			break;																								\
		}																										\
	}																											\
}

void KShaderProgram::CreateShadersFromParams(const FShaderProgramCreateParams& params)
{
	CreationParams = params;

#if _PACK
	static std::ofstream shaderList;
	static TVector<KString> addedShaders;

	shaderList.open("shaderlist.txt", std::ios::app | std::ios::out);

	if (!VectorContains(addedShaders, params.VertexShader.FileName))
	{
		shaderList << params.VertexShader.FileName + "\n";
		addedShaders.push_back(params.VertexShader.FileName);
	}

	if (!VectorContains(addedShaders, params.PixelShader.FileName))
	{
		shaderList << params.PixelShader.FileName + "\n";
		addedShaders.push_back(params.PixelShader.FileName);
	}

	if (!VectorContains(addedShaders, params.ComputeShader.FileName))
	{
		shaderList << params.ComputeShader.FileName + "\n";
		addedShaders.push_back(params.ComputeShader.FileName);
	}

	shaderList.close();

	return;
#endif

	if (params.VertexShader.LoadMethod != EShaderLoadMethod::Invalid)
	{																	
		VertexShader = CreateVertexShader();

		switch (params.VertexShader.LoadMethod)
		{
			case EShaderLoadMethod::FromFile:
			{
				CompileFromFile(EShaderStage::Vertex, params.VertexShader.FileName, VertexShader.get());
				break;
			}
			case EShaderLoadMethod::FromString:
			{
				CompileFromString(EShaderStage::Vertex, params.VertexShader.CodeString, VertexShader.get());
				break;
			}
			case EShaderLoadMethod::Precompiled:
			{
				FromPrecompiled(EShaderStage::Vertex, params.VertexShader.Data, params.VertexShader.Size, VertexShader.get());
				break;
			}
		}
	}

	if (params.PixelShader.LoadMethod != EShaderLoadMethod::Invalid)
	{
		PixelShader = CreatePixelShader();

		switch (params.PixelShader.LoadMethod)
		{
			case EShaderLoadMethod::FromFile:
			{
				CompileFromFile(EShaderStage::Pixel, params.PixelShader.FileName, PixelShader.get());
				break;
			}
			case EShaderLoadMethod::FromString:
			{
				CompileFromString(EShaderStage::Pixel, params.PixelShader.CodeString, PixelShader.get());
				break;
			}
			case EShaderLoadMethod::Precompiled:
			{
				FromPrecompiled(EShaderStage::Pixel, params.PixelShader.Data, params.PixelShader.Size, PixelShader.get());
				break;
			}
		}
	}

	if (params.ComputeShader.LoadMethod != EShaderLoadMethod::Invalid)
	{
		ComputeShader = CreateComputeShader();

		switch (params.ComputeShader.LoadMethod)
		{
			case EShaderLoadMethod::FromFile:
			{
				CompileFromFile(EShaderStage::Compute, params.ComputeShader.FileName, ComputeShader.get());
				break;
			}
			case EShaderLoadMethod::FromString:
			{
				CompileFromString(EShaderStage::Compute, params.ComputeShader.CodeString, ComputeShader.get());
				break;
			}
			case EShaderLoadMethod::Precompiled:
			{
				FromPrecompiled(EShaderStage::Compute, params.ComputeShader.Data, params.ComputeShader.Size, ComputeShader.get());
				break;
			}
		}
	}

	FinalizeProgram();
}

#undef ASSIGN_SHADER

#endif