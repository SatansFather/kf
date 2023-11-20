#pragma once

#if !_SERVER

enum class ECullMode : unsigned char
{
	CullNone = 1,
	CullFront = 2,
	CullBack = 3
};

enum class EFillMode : unsigned char
{
	Solid = 1,
	Wireframe = 2
};

class KRasterizerState
{
protected:

	EFillMode FillMode = EFillMode::Solid;
	ECullMode CullMode = ECullMode::CullNone;

public:

	virtual ~KRasterizerState() = default;

	/* virtual interface */
	virtual void SetFillMode(EFillMode mode) = 0;
	virtual void SetCullMode(ECullMode mode) = 0;

	inline EFillMode GetFillMode() { return FillMode; }
	inline ECullMode GetCullMode() { return CullMode; }

};

#endif