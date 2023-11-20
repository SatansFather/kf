#pragma once

#if !_SERVER

#include "kfglobal.h"

struct FColor8
{
	u32 Color;

	constexpr FColor8() : Color() {}
	constexpr FColor8(const FColor8& col) : Color(col.Color) {}
	constexpr FColor8(u32 col) : Color(col) {}
	constexpr FColor8(u8 r, u8 g, u8 b, u8 a) : Color((a << 24u) | (r << 16u) | (g << 8u) | b) {}
	constexpr FColor8(u8 r, u8 g, u8 b) : Color((r << 16u) | (g << 8u) | b) {}
	constexpr FColor8(FColor8 col, u8 a) : FColor8((a << 24u) | col.Color) {}

	FColor8& operator=(FColor8 color)
	{
		Color = color.Color;
		return *this;
	}
	constexpr u8 GetA() const
	{
		return Color >> 24u;
	}
	constexpr u8 GetR() const
	{
		return (Color >> 16u) & 0xFFu;
	}
	constexpr u8 GetG() const
	{
		return (Color >> 8u) & 0xFFu;
	}
	constexpr u8 GetB() const
	{
		return Color & 0xFFu;
	}
	void SetA(u8 a)
	{
		Color = (Color & 0xFFFFFFu) | (a << 24u);
	}
	void SetR(u8 r)
	{
		Color = (Color & 0xFF00FFFFu) | (r << 16u);
	}
	void SetG(u8 g)
	{
		Color = (Color & 0xFFFF00FFu) | (g << 8u);
	}
	void SetB(u8 b)
	{
		Color = (Color & 0xFFFFFF00u) | b;
	}

	struct FColor32 To32() const;
	FColor8 Lerp(FColor8 other, f32 alpha) const;
};

struct FColor32
{
	f32 r = 0, g = 0, b = 0, a = 1.f;

	FColor32() = default;
	FColor32(f32 r, f32 g, f32 b, f32 a) : r(r), g(g), b(b), a(a) {}

	FColor8 To8() const;
	FColor32 Inverse() const;
	FColor32 Lerp(const FColor32& other, f32 alpha) const;
};

#endif