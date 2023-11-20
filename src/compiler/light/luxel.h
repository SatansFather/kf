#pragma once

#include "engine/math/vec3.h"

// luxel is a lightmap texel
class KLuxel
{
	enum
	{
		Shadowed         = 1,
		NeedRadius       = 2,
		OnPoly           = 4, // point is on poly but may have been moved to it
		OriginallyOnPoly = 8, // point was on poly from the start
		OnPolyEdge       = 16,
	};

	u8 Flags = 0;

public:

	DVec3 Color = DVec3(0, 0, 0);
	DVec3 GIScale = DVec3(1, 1, 1);
	DVec2 Point2D;
	DVec3 Point3D;
	DVec3 Normal;
	u32 EdgeIndex = MAX_U32; // nearest edge index
	f64 EdgeDistance = -1;   // nearest edge distance

	TVector<class KLightEntity*> AffectingLights;

	void SetShadowed(bool shadowed)
	{
		if (shadowed) Flags |= Shadowed;
		else          Flags &= ~Shadowed;
	}
	
	void SetNeedRadius(bool radius)
	{
		if (radius) Flags |= NeedRadius;
		else        Flags &= ~NeedRadius;
	}

	void SetOnPoly(bool onPoly)
	{
		if (onPoly) Flags |= OnPoly;
		else        Flags &= ~OnPoly;
	}

	void SetOriginallyOnPoly(bool onPoly)
	{
		if (onPoly) Flags |= OriginallyOnPoly;
		else        Flags &= ~OriginallyOnPoly;
	}

	void SetOnPolyEdge(bool polyEdge)
	{
		if (polyEdge) Flags |= OnPolyEdge;
		else          Flags &= ~OnPolyEdge;
	}

	bool IsShadowed() const { return Flags & Shadowed; }
	bool DoesNeedRadius() const { return Flags & NeedRadius; }
	bool IsOnPoly() const { return Flags & OnPoly; }
	bool IsOriginallyOnPoly() const { return Flags & OriginallyOnPoly; }
	bool IsOnPolyEdge() const { return Flags & OnPolyEdge; }
};