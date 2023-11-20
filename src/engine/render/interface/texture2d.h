#pragma once

#if !_SERVER

#include "../surface2d.h"
#include "engine/math/glm.h"

class KTexture2D
{
protected:

	UPtr<KSurface2D> Surface;
	//KString Name;

public:

	KTexture2D() = default;
	virtual ~KTexture2D() = default;

	//const KString& GetName() const { return Name; }
	KSurface2D* GetSurface()  { return Surface.get(); }

	// frees the memory used by the surface, safe to do once the texture has been created on the GPU
	// cannot be undone
	void FreeSurface() { Surface.reset(); }

	virtual void CreateFromSurface(UPtr<KSurface2D>& surface) = 0;
	virtual void CreateFrom2dFloatArray(f32* values, u32 w, u32 h) = 0;
	virtual void CreateFromFreetypeGlyph(struct FT_GlyphSlotRec_* glyph) = 0;
};

#endif