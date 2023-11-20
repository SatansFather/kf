#pragma once

#include "engine/math/vec3.h"

// luxel is a lightmap texel
class KFaceAttribs
{
public:

	enum FaceAttribsFlags
	{
		MaskedShadows = 1,
		MaskedShadowColoring = 2,
	};

	struct GI_Mod
	{
		enum GI_Mod_Flags
		{
			Ignore = 1,
			Unlit = 2
		};
		u8 ModFlags = 0;
		f32 R = 1, G = 1, B = 1, A = 1;
	};

	f32 RedIntensity = 1, GreenIntensity = 1, BlueIntensity = 1;
	f32 RedPass = 0, GreenPass = 0, BluePass = 0;
	f32 RedGISkyScale = 1, GreenGISkyScale = 1, BlueGISkyScale = 1, AlphaGISkyScale = 1;
	f32 RedGIFogScale = 1, GreenGIFogScale = 1, BlueGIFogScale = 1, AlphaGIFogScale = 1;
	f32 RedDiffuseShadow = 0, GreenDiffuseShadow = 0, BlueDiffuseShadow = 0;
	u8 Flags = MaskedShadows & MaskedShadowColoring;
	f32 LightPositionOffset = 1;
	TVector<i32> IgnoreFrom, IgnoreTo;
	TMap<i32, GI_Mod> GIMod;
	TVector<i32> ExtraSmoothingGroups;
	KString Shader;

	// raw property data, intended for material properties
	TMap<KString, KString> PropertyMap;

	static KFaceAttribs* CreateNewAttribs(class KMapEntity* ent);
};
