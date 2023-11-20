import os

headers = []

print("generating shader loading code")

# read LoadShaders in render interface
loadShaders = []
with open ("src/engine/render/interface/render_interface.cpp", "r") as file:
	insideLoadFunction = False
	scopeCount = 0
	hasScoped = False
	endOfFunction = False
	for line in file:
		if "void KRenderInterface::LoadShaders()" in line:
			insideLoadFunction = True
			continue
		if insideLoadFunction:
			dontAppend = False
			for char in line:
				if char == '{':
					if not hasScoped:
						dontAppend = True
						hasScoped = True
					scopeCount += 1
				elif char == '}':
					scopeCount -= 1
					if hasScoped and scopeCount == 0:
						dontAppend = True
						endOfFunction = True
						break;
			if not dontAppend:
				loadShaders.append(line)
			if endOfFunction:
				break;

# adjust function to load bytecode instead of shader files

# replace "shader_vs" with HLSL_shader_vs
funcStr = ""
for line in loadShaders:
	newLine = line
	if "FileName" in line:
		shaderVarName = ""
		shaderType = ""
		newLine = newLine.replace("FileName", "Data")
		newLine = newLine.replace("\"", "HLSL_")
		newLine = newLine.replace("HLSL_;\n", ";")

		shaderVarName = newLine.split("=")[1].split(";")[0].strip()
			
		if shaderVarName not in headers:
			headers.append(shaderVarName)

		if "VertexShader" in newLine:
			shaderType = "VertexShader"
		elif "PixelShader" in newLine:
			shaderType = "PixelShader"
		elif "ComputeShader" in newLine:
			shaderType = "ComputeShader"
		newLine += "\n\t\tparams." + shaderType + ".Size = sizeof(" + shaderVarName + ");\n"
		
		newLine = newLine.replace("Data = ", "Data = (void*)")

	if "FromFile" in newLine:
		newLine = newLine.replace("FromFile", "Precompiled")
		
	funcStr += newLine

fileStr = """#if !_SERVER && _WIN32

#include "d3d11_interface.h"
#include "shader_include.h"
#include "../../input_layout.h"
#include "../../interface/shader_program.h"

#if _DEV
void KRenderInterface_D3D11::LoadShaders()
{
	KRenderInterface::LoadShaders();
}
#else
void KRenderInterface_D3D11::LoadShaders()
{
"""

fileStr += funcStr

fileStr += """
}

#endif
#endif
"""

with open("src/engine/render/backend/d3d11/d3d11_shader_load.cpp", "w") as file:
	file.write(fileStr)
	
with open ("src/engine/render/backend/d3d11/shader_include.h", "w") as file:
	file.writelines("#pragma once\n")
	file.writelines("#if !_DEV\n")
	for h in headers:
		file.writelines("#include \"hlsl/compiled/" + h + ".h\"\n")
	file.writelines("#endif\n")

print("finished")