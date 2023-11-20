#if !_SERVER

#include "engine/math/math.h"
#include "color.h"

FColor32 FColor8::To32() const
{
	FColor32 col;

	col.r = (f32)GetR() / 255.f;
	col.g = (f32)GetG() / 255.f;
	col.b = (f32)GetB() / 255.f;
	col.a = (f32)GetA() / 255.f;

	return col;
}

FColor8 FColor8::Lerp(FColor8 other, f32 alpha) const
{
	return FColor8(
		KLerp(GetR(), other.GetR(), alpha), 
		KLerp(GetG(), other.GetG(), alpha), 
		KLerp(GetB(), other.GetB(), alpha), 
		KLerp(GetA(), other.GetA(), alpha));
}

FColor8 FColor32::To8() const
{
	FColor8 col;

	col.SetR((u8)(KSaturate(r) * 255));
	col.SetG((u8)(KSaturate(g) * 255));
	col.SetB((u8)(KSaturate(b) * 255));
	col.SetA((u8)(KSaturate(a) * 255));

	return col;
}

FColor32 FColor32::Inverse() const
{
	return FColor32(1 - r, 1 - g, 1 - b, a);
}

FColor32 FColor32::Lerp(const FColor32& other, f32 alpha) const
{
	return FColor32(
		KLerp(r, other.r, alpha), 
		KLerp(g, other.g, alpha), 
		KLerp(b, other.b, alpha), 
		KLerp(a, other.a, alpha));
}

#endif