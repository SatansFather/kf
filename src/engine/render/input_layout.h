#pragma once

#if !_SERVER

#include "kfglobal.h"

enum class EInputElementType : u8
{
	Float,
	Int32,
	UInt32,
	Int16,
	UInt16,
	Int8,
	UInt8,
	Mat4x4,
};

static TMap<EInputElementType, u32> InputElementSize = 
{
	{ EInputElementType::Float,  4  },
	{ EInputElementType::Int32,  4  },
	{ EInputElementType::UInt32, 4  },
	{ EInputElementType::Int16,  2  },
	{ EInputElementType::UInt16, 2  },
	{ EInputElementType::Int8,	 1  },
	{ EInputElementType::UInt8,  1  },
	{ EInputElementType::Mat4x4, 64 }
};

enum class EInputSlotClass : unsigned char
{
	PerVertex,
	PerInstance
};

struct FInputElement
{
	FInputElement() = default;
	FInputElement(const KString& name, EInputElementType type, u8 count, EInputSlotClass slotClass, u32 semanticIndex = 0)
		: SemanticName(name), Type(type), Count(count), Class(slotClass), SemanticIndex(semanticIndex) {}

	KString SemanticName;
	EInputElementType Type;
	u8 Count;
	EInputSlotClass Class;
	u32 SemanticIndex;
};

class FInputLayout
{
public:

	// a description of each vertex
	TVector<FInputElement> Elements;
};

#endif