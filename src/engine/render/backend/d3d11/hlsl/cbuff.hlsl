#ifndef CBUFF
#define CBUFF

#include "../../../bufferslots.h"
#include "../../../buffers/buffer_fog.h"

cbuffer CB_ModelViewProjection : CB_REGISTER(CB_MVP)
{
	float4x4 ModelViewProjection;
}

cbuffer CB_InvView : CB_REGISTER(CB_VIEW)
{
	float4x4 View;
	float4x4 Projection;
	float4x4 SpriteView;
}

cbuffer CB_ModelTransform : CB_REGISTER(CB_MODEL)
{
	float4x4 ModelTransform;
}

cbuffer CB_LightMatrix : CB_REGISTER(CB_LIGHTMAT)
{
	float4x4 LightSpaceMatrix;
}

cbuffer CB_TimeData : CB_REGISTER(CB_FRAME)
{
	float Time;
	float CamPitch;
	float RenderAlpha;
	float _timedatapad;
	float2 Resolution;
	float2 _timedatapad2;
	uint ActiveFlashCount;
	uint3 __flash_pad;
	uint ActiveDynamicLightCount;
	uint3 __flash_pad1;
	float LastTeleportTime;
	float3 __tele_pad;
	float UnderWaterDepth;
	float3 __water_pad;
	float RenderScale;
	float3 __rscale_pad;
	float4 ScreenAdjust;
}

cbuffer CB_CameraData : CB_REGISTER(CB_CAMERA)
{
	float4 CamPosition;
	float4 CamDirection;
	float  ConcussionStrength;
	float3 _camdatapad0;
	float4 UnderWaterColor;
}

cbuffer CB_Fog : CB_REGISTER(CB_FOG)
{
	uint ActiveFogCount;
	uint3 __FOG_PAD;
	KFogBuffer ActiveFog[32];
}

cbuffer CB_LightFlash : CB_REGISTER(CB_LIGHTFLASH)
{
	KLightFlash ActiveFlashes[MAX_FLASHES];
}

#endif