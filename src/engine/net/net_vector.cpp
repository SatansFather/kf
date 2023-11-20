#include "net_vector.h"

KNetVec3::KNetVec3(const GVec3& v, bool tele)
{
	// cap at +/- 32k
	x = std::clamp(i32(v.x), (i32)MIN_I16, (i32)MAX_I16);
	y = std::clamp(i32(v.y), (i32)MIN_I16, (i32)MAX_I16);
	z = std::clamp(i32(v.z), (i32)MIN_I16, (i32)MAX_I16);

	// 5 bits each decimal
	for (i32 i = 0; i < 3; i++)
	{
		const GFlt frac = v[i] - i32(v[i]);
		const u16 fracDenom = 0b0000000000011111;
		const u16 ifrac = frac * fracDenom;
		Data |= (fracDenom & ifrac) << (i * 5);
	}

	// last bit for teleport flag
	if (tele) Data |= 0x8000;
}

GVec3 KNetVec3::ToVec3() const
{
	GVec3 v(x, y, z);

	for (i32 i = 0; i < 3; i++)
	{
		const u16 fracDenom = 0b0000000000011111;

		u16 frac = Data & (fracDenom << (i * 5));
		frac >>= (i * 5);
		GFlt f = (GFlt)frac / (GFlt)fracDenom;
		v[i] += f;
	}
	return v;
}

KNetVec3_Normal::KNetVec3_Normal(const GVec3& v)
{
	x = v.x * MAX_I8;
	y = v.y * MAX_I8;
	z = v.z * MAX_I8;
}

GVec3 KNetVec3_Normal::ToVec3() const
{
	GVec3 out;
	out.x = GFlt(x) / GFlt(MAX_I8);
	out.y = GFlt(y) / GFlt(MAX_I8);
	out.z = GFlt(z) / GFlt(MAX_I8);
	return out;
}
