#if !_SERVER && _WIN32

#include "d3d11_shader.h"

// look up DGXI format based on input element data
TMap<EInputElementType, TMap<u8, DXGI_FORMAT>> FormatMap
{
	{ 
		EInputElementType::Float,
		{ 
			{ 1,          DXGI_FORMAT_R32_FLOAT },
			{ 2,       DXGI_FORMAT_R32G32_FLOAT },
			{ 3,    DXGI_FORMAT_R32G32B32_FLOAT },
			{ 4, DXGI_FORMAT_R32G32B32A32_FLOAT },
		}
	},

	{
		EInputElementType::Int32,
		{
			{ 1,          DXGI_FORMAT_R32_SINT },
			{ 2,       DXGI_FORMAT_R32G32_SINT },
			{ 3,    DXGI_FORMAT_R32G32B32_SINT },
			{ 4, DXGI_FORMAT_R32G32B32A32_SINT },
		}
	},

	{
		EInputElementType::UInt32,
		{
			{ 1,          DXGI_FORMAT_R32_UINT },
			{ 2,       DXGI_FORMAT_R32G32_UINT },
			{ 3,    DXGI_FORMAT_R32G32B32_UINT },
			{ 4, DXGI_FORMAT_R32G32B32A32_UINT },
		}
	},

	{
		EInputElementType::Int16,
		{
			{ 1,          DXGI_FORMAT_R16_SINT },
			{ 2,       DXGI_FORMAT_R16G16_SINT },
			{ 4, DXGI_FORMAT_R16G16B16A16_SINT },
		}
	},

	{
		EInputElementType::UInt16,
		{
			{ 1,          DXGI_FORMAT_R16_UINT },
			{ 2,       DXGI_FORMAT_R16G16_UINT },
			{ 4, DXGI_FORMAT_R16G16B16A16_UINT },
		}
	},

	{
		EInputElementType::Int8,
		{
			{ 1,       DXGI_FORMAT_R8_SINT },
			{ 2,     DXGI_FORMAT_R8G8_SINT },
			{ 4, DXGI_FORMAT_R8G8B8A8_SINT },
		}
	},

	{
		EInputElementType::UInt8,
		{
			{ 1,       DXGI_FORMAT_R8_UINT },
			{ 2,     DXGI_FORMAT_R8G8_UINT },
			{ 4, DXGI_FORMAT_R8G8B8A8_UINT },
		}
	},
};

void KShaderVertex_D3D11::SetInputLayout(const struct FInputLayout& layout)
{
	if (layout.Elements.size() == 0) return;

	TVector<D3D11_INPUT_ELEMENT_DESC> inputlayout;

	u32 instanceSize = 0;

	for (i32 i = 0; i < layout.Elements.size(); i++)
	{
		const FInputElement& element = layout.Elements[i];
		const bool instance = element.Class == EInputSlotClass::PerInstance;

		D3D11_INPUT_ELEMENT_DESC e;
		e.SemanticName = element.SemanticName;
		e.SemanticIndex = element.SemanticIndex;
		e.Format = FormatMap[element.Type][element.Count];
		e.InputSlot = instance ? 1 : 0;
		e.AlignedByteOffset = instance ? instanceSize : D3D11_APPEND_ALIGNED_ELEMENT;

		if (instance)
			instanceSize += InputElementSize[element.Type] * element.Count;

		e.InputSlotClass = instance ?
			D3D11_INPUT_PER_INSTANCE_DATA :
			D3D11_INPUT_PER_VERTEX_DATA;

		e.InstanceDataStepRate = instance ? 1 : 0;

		inputlayout.push_back(e);
	}

	GetDevice()->CreateInputLayout(inputlayout.data(), u8(inputlayout.size()), ShaderCode, CodeSize, &InputLayout);
}

#endif

