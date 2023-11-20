#pragma once

#include "kfglobal.h"
#include "math.h"

template <typename IntType>
struct KPackedFloat
{
	static_assert(std::is_unsigned<IntType>::value, "Type must be an unsigned integer");

	IntType Value;

	KPackedFloat() = default;

	KPackedFloat(IntType i) : Value(i) {}

	template <typename FltType = f32>
	KPackedFloat(FltType flt, u8 fracBits)
	{
		PackFloat(flt, fracBits);
	}

	// returns false if the int value had to be clamped
	template <typename FltType>
	bool PackFloat(FltType flt, u8 fracBits)
	{
		// size in bits
		u8 typeSize = sizeof(IntType) * 8;

		K_ASSERT(fracBits < typeSize - 1, "Packed Float requires one bit for sign and at least one bit for integer");

		Value = 0;

		const u8 intBits = typeSize - fracBits;
		const IntType maxInt = std::pow(2, intBits - 1) - 1;
		const IntType maxDec = std::pow(2, fracBits) - 1;
		const bool pos = flt >= 0;

		FltType aflt = abs(flt);

		const FltType frac = aflt - (IntType)aflt;
		IntType i = std::round(aflt - frac);
		bool needClamp = i > maxInt;
		if (needClamp) i = maxInt;
		
		IntType f = MapRange(frac, 0, 1, 0, maxDec + 1);

		if (pos) Value = 1 << (typeSize - 1);
		Value |= i << fracBits;
		Value |= f;

		return !needClamp;
	}

	template <typename FltType = f32>
	FltType UnpackFloat(u8 fracBits)
	{
		// size in bits
		u8 typeSize = sizeof(IntType) * 8;

		const u8 intBits = typeSize - fracBits;
		const IntType fullInt = (std::numeric_limits<IntType>::max)();
		const IntType maxInt = std::pow(2, intBits - 1) - 1;
		const IntType maxDec = std::pow(2, fracBits) - 1;

		IntType ival = maxInt << fracBits;
		ival &= Value;
		ival >>= fracBits;
		IntType fval = Value & maxDec;

		FltType frac = MapRange(fval, 0, maxDec + 1, 0, 1);

		IntType sign = Value >> (typeSize - 1);

		return (ival + frac) * (sign == 0 ? -1 : 1); 
	}
};

typedef KPackedFloat<u8> KPackedFloat8;
typedef KPackedFloat<u16> KPackedFloat16;