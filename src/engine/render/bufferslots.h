// common file between C++ and HLSL

#if !_SERVER
#ifndef BUFFER_SLOTS
#define BUFFER_SLOTS

#ifdef __cplusplus
#include "kfglobal.h"
#include "engine/math/glm.h"
#include "engine/math/vec3.h"
#include "render_buffer_info.h"
#include "color.h"
#include "interface/texture2d.h"
#include "model_formats/model_obj.h"
#endif

#ifdef __cplusplus
#define FLOAT4	glm::vec4
#define UINT4	glm::uvec4
#define gUINT	u32
#define MAT4	glm::mat4
#else
#define FLOAT4	float4
#define UINT4	uint4
#define gUINT	uint
#define MAT4	float4x4
#endif

// constant buffer slots
#define CB_MVP		0	// modeltransform * view matrix * projection matrix
#define CB_MODEL	1	// just the "M" in "MVP"
#define CB_LIGHTMAT 2	// view matrix for shadowmap/lightdepth generation
#define CB_FRAME	4	// passage of time
#define CB_VIEW		5	// inverse view matrix
#define CB_CAMERA	6	// camera position and direction
#define CB_PRECIP	7	// precipitation 
#define CB_FOG		8	// fog data

#define CB_SPOTLIGHT	9
#define CB_POINTLIGHT	10
#define CB_PARALIGHT	11

#define CB_LIGHTFLASH		12	// light flashes
#define MAX_FLASHES			32

#define SB_DYNAMIC_LIGHTS	64

// sampler slots
#define SMP_UNFILTERED		0
#define SMP_FILTERED		1
#define SMP_CONFIG			2	// if theres an option to enable/disable texture filtering
#define SMP_SHADOWMAP		3	// samples a light's depth buffer for depth comparison
#define SMP_RENDER_SCALE	5	// render scale smooth or not

#define CB_REGISTER(x) register(b##x)
#define T_REGISTER(x) register(t##x)
#define SMP_REGISTER(x) register(s##x)

#define MAT_BUFFER_SLOT 5

// particle buffer sizes
#define PB_TEST			10000

struct KPortalData
{
	FLOAT4 Colors;
	FLOAT4 Data;

#ifdef __cplusplus
	void SetTimeScale(float scale)     { Data.x = scale; }
	void SetRippleRate(float rate)     { Data.y = rate; }
	void SetHeightRatio(float ratio)   { Data.z = ratio; }
	void SetBackColor(FColor8 color)   { memcpy(&Colors.x, &color.Color, 4); }
	void SetRippleColor(FColor8 color) { memcpy(&Colors.y, &color.Color, 4); }

	void InitFromMap(TMap<KString, KString>& m);

#else
	float GetTimeScale()   { return Data.x; }
	float GetRippleRate()  { return Data.y; }
	float GetHeightRatio() { return Data.z; }

	float4 GetBackColor()
	{
		float4 color;
		uint col = asuint(Colors.x);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
	float4 GetRippleColor()
	{
		float4 color;
		uint col = asuint(Colors.y);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
#endif
};


struct KWaterData
{
	FLOAT4 Data;
	FLOAT4 Data1;

#ifdef __cplusplus
	void SetWaterColor(FColor8 color) { memcpy(&Data.x, &color.Color, 4); }
	void SetCaustic1(FColor8 color) { memcpy(&Data.y, &color.Color, 4); }
	void SetCaustic2(FColor8 color) { memcpy(&Data.z, &color.Color, 4); }
	void SetMaxDepth(float depth) { Data.w = depth; }
	void SetWaterizeScale(float scale) { Data1.x = scale; }

	void InitFromMap(TMap<KString, KString>& m);

#else

	float GetMaxDepth() { return Data.w; }
	float GetWaterizeScale() { return Data1.x; }

	float4 GetWaterColor()
	{
		float4 color;
		uint col = asuint(Data.x);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
	float4 GetCaustic1()
	{
		float4 color;
		uint col = asuint(Data.y);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
	float4 GetCaustic2()
	{
		float4 color;
		uint col = asuint(Data.z);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
#endif
};

struct KBoundingBoxRender
{
	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;
	FLOAT4 HalfExtent;
	FLOAT4 Color;

#ifdef __cplusplus
	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);
#endif

};

struct KJohnnyJiantdick
{
	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;

#ifdef __cplusplus

	static UPtr<class KTexture2D> JohnnyPic;

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const GVec3& pos)
	{
		PrevPos.x = pos.y;
		PrevPos.y = pos.z;
		PrevPos.z = pos.x;
	}

	void SetCurrentPosition(const GVec3& pos)
	{
		CurrentPos.x = pos.y;
		CurrentPos.y = pos.z;
		CurrentPos.z = pos.x;
	}

	void SetLastMoveAlpha(f32 alpha) { CurrentPos.w = alpha; }
#else
	float3 GetPosition(float alpha)
	{
		return lerp(PrevPos.xyz, CurrentPos.xyz, saturate(alpha) / CurrentPos.w + PrevPos.w);
	}
#endif
};

struct KShotgunShard
{
	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const GVec3& pos)
	{
		PrevPos.x = pos.y;
		PrevPos.y = pos.z;
		PrevPos.z = pos.x;
	}

	void SetCurrentPosition(const GVec3& pos)
	{
		CurrentPos.x = pos.y;
		CurrentPos.y = pos.z;
		CurrentPos.z = pos.x;
	}

	void SetRandomSeed(f32 seed) { PrevPos.w = seed; }
	void SetLastMoveAlpha(f32 alpha) { CurrentPos.w = alpha; }

#else

	float3 GetPrevPos() { return PrevPos.xyz; }
	float3 GetCurrentPos() { return CurrentPos.xyz; }
	float GetRandomSeed() { return PrevPos.w; }
	float GetLastMoveAlpha() { return CurrentPos.w; }

#endif
};

#ifdef __cplusplus
struct KPrecipitationBuffer
#else
cbuffer KPrecipitationBuffer : CB_REGISTER(CB_PRECIP)
#endif
{
#ifdef __cplusplus
	enum { Rain, Snow, Hail };

	//UPtr<class KTexture2D> StopPoints;
	UPtr<class KGpuBuffer> StopPoints;
	UPtr<class KGpuBuffer> Buffer;
	UPtr<class KVertexBuffer> IndexBuffer;
	class KShaderProgram* Shader = nullptr;

	bool bInitialized = false;

	~KPrecipitationBuffer();

	void Init(const class KPrecipitationVolume& p);
	void Draw();

#endif

	FLOAT4 PrecipMin;
	FLOAT4 PrecipMax;
	FLOAT4 PrecipData;
};

struct SmokeBeamData
{
#ifdef __cplusplus
	typedef u32 uint;
#endif	

	uint x, y;
	float z, w;
};

struct KSmokeBeam
{
#define SMOKE_BEAM_MAX_PARTICLES 512000

	SmokeBeamData Data;
	FLOAT4 StartPos;
	FLOAT4 LastEndPos;
	FLOAT4 EndPos;

#ifdef __cplusplus
	static UPtr<class KTexture2D> Tex1, Tex2;
	static UPtr<KGpuBuffer> IndexMapBuffer;
	static TVector<u32> CpuIndexMap;
	static TVector<u32> GpuIndexMap;
	static u32 CpuParticleCount; 
	static u32 BridgeParticleCount; 
	static u32 GpuParticleCount; 
	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetStartPos(const GVec3& pos)
	{
		StartPos.x = pos.y;
		StartPos.y = pos.z;
		StartPos.z = pos.x;
	}

	void SetEndPos(const GVec3& pos)
	{
		EndPos.x = pos.y;
		EndPos.y = pos.z;
		EndPos.z = pos.x;
	}

	void SetLastEndPos(const GVec3& pos)
	{
		LastEndPos.x = pos.y;
		LastEndPos.y = pos.z;
		LastEndPos.z = pos.x;
	}

	void SetColor(FColor8 color) { memcpy(&Data.x, &color.Color, 4); }
	void SetSpacing(f32 spacing) { StartPos.w = spacing; }
	void SetRadius(f32 radius) { EndPos.w = radius; }
	void SetStartIndex(u32 index) { Data.y = index; }
	void SetTimeOffset(f32 time) { Data.z = time; }
	void SetAge(f32 time) { Data.w = time; }
#else
	float4 GetColor()
	{
		float4 color;
		uint col = asuint(Data.x);
		color.a =  (col >> 24)		   / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >>  8) & 0xFF) / 255.f;
		color.b =  (col        & 0xFF) / 255.f;
		return color;
	}
	
	float3 GetStartPos() { return StartPos.xyz; }
	float3 GetEndPos() { return EndPos.xyz; }
	float3 GetLastEndPos() { return LastEndPos.xyz; }
	float GetRadius() { return EndPos.w; }
	float GetSpacing() { return StartPos.w; }
	uint GetStartIndex() { return Data.y; }
	float GetTimeOffset() { return Data.z; }
	float GetAge() { return Data.w; }
#endif

};

struct KHitSpark
{
#define HIT_SPARK_COUNT 12

	FLOAT4 HitPosition;
	FLOAT4 HitNormal;

#ifdef __cplusplus
	
	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		HitPosition.x = pos.y;
		HitPosition.y = pos.z;
		HitPosition.z = pos.x;
	}

	void SetNormal(const FVec3& norm)
	{
		HitNormal.x = norm.y;
		HitNormal.y = norm.z;
		HitNormal.z = norm.x;
	}

	void SetTimeCreated(f32 time) { HitPosition.w = time; }
	void SetRandomOffset(f32 offset) { HitNormal.w = offset; }
#else
	float GetTimeCreated() { return HitPosition.w; }
	float GetRandomOffset() { return HitNormal.w; }
	float3 GetHitPosition() { return HitPosition.xyz; }
	float3 GetHitNormal() { return HitNormal.xyz; }
#endif	
};

struct KWaterSplash
{
#define WATER_SPLASH_COUNT 128

	FLOAT4 HitPosition;
	FLOAT4 HitNormal;
	FLOAT4 Data;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		HitPosition.x = pos.y;
		HitPosition.y = pos.z;
		HitPosition.z = pos.x;
	}

	void SetNormal(const FVec3& norm)
	{
		HitNormal.x = norm.y;
		HitNormal.y = norm.z;
		HitNormal.z = norm.x;
	}

	void SetTimeCreated(f32 time) { HitPosition.w = time; }
	void SetRandomOffset(f32 offset) { HitNormal.w = offset; }
	void SetColor(FColor8 color) { memcpy(&Data.x, &color.Color, 4); }
	void SetStrength(f32 strength) { Data.y = strength; }
	void SetScale(f32 scale) { Data.z = scale; }
	
#else
	float GetTimeCreated() { return HitPosition.w; }
	float GetRandomOffset() { return HitNormal.w; }
	float3 GetHitPosition() { return HitPosition.xyz; }
	float3 GetHitNormal() { return HitNormal.xyz; }

	float4 GetColor()
	{
		float4 color;
		uint col = asuint(Data.x);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}

	float GetStrength() { return Data.y; }
	float GetScale() { return Data.z; }
	
#endif	
};

struct KLightFlash
{
	FLOAT4 Data;
	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;

#ifdef __cplusplus
	
	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const GVec3& pos)
	{
		PrevPos.x = pos.y;
		PrevPos.y = pos.z;
		PrevPos.z = pos.x;
	}
	void SetCurrentPosition(const GVec3& pos)
	{
		CurrentPos.x = pos.y;
		CurrentPos.y = pos.z;
		CurrentPos.z = pos.x;
	}
	void SetTimeCreated(f32 time) { Data.x = time; }
	void SetColor(FColor8 color) { memcpy(&Data.y, &color.Color, 4); }
	void SetGrowDuration(f32 duration) { Data.z = duration; }
	void SetDimDuration(f32 duration) { Data.w = duration; }
	void SetRadius(f32 radius) { CurrentPos.w = radius; }
	void SetFalloff(f32 falloff) { PrevPos.w = falloff; }
#else
	float GetTimeCreated() { return Data.x; }
	float GetGrowDuration() { return Data.z; }
	float GetDimDuration() { return Data.w; }
	float GetRadius() { return CurrentPos.w; }
	float GetFalloff() { return PrevPos.w; }

	float3 GetPosition(float alpha) 
	{ 
		return lerp(PrevPos.xyz, CurrentPos.xyz, alpha); 
	}

	
	float4 GetColor()
	{
		float4 color;
		uint col = asuint(Data.y);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}

#endif
};

#ifdef __cplusplus
struct KFlashBufferCount // CB_LIGHTFLASH_COUNT
{
	u32 FlashCount;
	u32 pad[3];
};
#endif

struct KDynamicLight
{
	FLOAT4 Data;
	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;

#ifdef __cplusplus

	KDynamicLight() { Data = glm::vec4(0, 0, 0, 0); }

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const GVec3& pos)
	{
		PrevPos.x = pos.y;
		PrevPos.y = pos.z;
		PrevPos.z = pos.x;
	}
	void SetCurrentPosition(const GVec3& pos)
	{
		CurrentPos.x = pos.y;
		CurrentPos.y = pos.z;
		CurrentPos.z = pos.x;
	}

	void SetColor(FColor8 color) { memcpy(&Data.x, &color.Color, 4); }
	void SetCurrentRadius(f32 radius) { CurrentPos.w = radius; }
	void SetPrevRadius(f32 radius) { PrevPos.w = radius; }
	void SetFalloff(f32 falloff) { Data.y = falloff; }

	// lights all exposed surfaces fully, regardless of normal
	void SetLightAll()
	{
		Data.z = 1;
	}

	// negative light
	void SetNegative()
	{
		Data.w = 1;
	}

#else
	float GetRadius(float alpha) { return lerp(PrevPos.w, CurrentPos.w, alpha); }
	float GetFalloff() { return Data.y; }
	float3 GetPosition(float alpha)	{ return lerp(PrevPos.xyz, CurrentPos.xyz, alpha); }

	bool IsFullFill()
	{
		return Data.z == 1;
	}

	bool IsNegative()
	{
		return Data.w == 1;
	}

	float4 GetColor()
	{
		float4 color;
		uint col = asuint(Data.x);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color * (IsNegative() ? -1 : 1);
	}

#endif
};

#ifdef __cplusplus

struct KStaticMeshBase
{
	MAT4 PrevModelMat;
	MAT4 CurrentModelMat;
	FLOAT4 Data;

	void SetTeleported() { Data.x = 1; }
	void SetLastMoveRenderAlpha(f32 alpha) { Data.y = alpha; }

	// allow these to be implemented outside of this file
	static void MeshRenderBuffer(const KRenderBufferInfo& info, class KModelObj* model, KTexture2D* tex);
	static void MeshCreateBuffers(const char* meshName, const char* texName, UPtr<KModelObj>& model, UPtr<KTexture2D>& texture, KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
};

// takes string literal for model name, loads it in CreateBuffers
template <StringLiteral ModelName, StringLiteral TexName>
struct KStaticMesh : public KStaticMeshBase
{
	static UPtr<KModelObj> Model;
	static UPtr<KTexture2D> Texture;

	static void RenderBuffer(const KRenderBufferInfo& info) { MeshRenderBuffer(info, Model.get(), Texture.get()); }
	static bool ShouldBeUpdated(const KRenderBufferInfo& info) { return true; }
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
	{
		MeshCreateBuffers(ModelName, TexName, Model, Texture, info, typeSize, elementCount);
	}
};

template <StringLiteral ModelName, StringLiteral TexName>
UPtr<KModelObj> KStaticMesh<ModelName, TexName>::Model;

template <StringLiteral ModelName, StringLiteral TexName>
UPtr<KTexture2D> KStaticMesh<ModelName, TexName>::Texture;
/*

#pragma region ViewWeapon

struct KStaticMeshBase_ViewWeapon
{
	MAT4 PrevModelMat;
	MAT4 CurrentModelMat;
	FLOAT4 Data;

	void SetTeleported() { Data.x = 1; }
	void SetLastMoveRenderAlpha(f32 alpha) { Data.y = alpha; }

	// allow these to be implemented outside of this file
	static void MeshRenderBuffer(const KRenderBufferInfo& info, class KModelObj* model, KTexture2D* tex);
	static void MeshCreateBuffers(const char* meshName, const char* texName, class KModelObj*& model, UPtr<KTexture2D>& texture, KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
};

template <StringLiteral ModelName, StringLiteral TexName>
struct KStaticMesh_ViewWeapon : public KStaticMeshBase_ViewWeapon
{
	static class KModelObj* Model;
	static UPtr<KTexture2D> Texture;

	static void RenderBuffer(const KRenderBufferInfo& info) { MeshRenderBuffer(info, Model, Texture.get()); }
	static bool ShouldBeUpdated(const KRenderBufferInfo& info) { return true; }
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount)
	{
		MeshCreateBuffers(ModelName, TexName, Model, Texture, info, typeSize, elementCount);
	}
};

template <StringLiteral ModelName, StringLiteral TexName>
class KModelObj* KStaticMesh_ViewWeapon<ModelName, TexName>::Model;

template <StringLiteral ModelName, StringLiteral TexName>
UPtr<KTexture2D> KStaticMesh_ViewWeapon<ModelName, TexName>::Texture;

#pragma endregion
*/

#else
struct KStaticMesh
{
	MAT4 PrevModelMat;
	MAT4 CurrentModelMat;
	FLOAT4 Data;

	bool Teleported() { return Data.x > 0; }
	float GetLastMoveRenderAlpha() { return Data.y; }
};
#endif

struct KBulletHole
{
	FLOAT4 Position;
	FLOAT4 Normal;

#ifdef __cplusplus

	static UPtr<KTexture2D> Texture;

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetNormal(const FVec3& norm)
	{
		Normal.x = norm.y;
		Normal.y = norm.z;
		Normal.z = norm.x;
	}

	void SetTimeCreated(f32 time) { Position.w = time; }
	void SetRandomOffset(f32 offset) { Normal.w = offset; }
#else
	float GetTimeCreated() { return Position.w; }
	float GetRandomOffset() { return Normal.w; }
	float3 GetPosition() { return Position.xyz; }
	float3 GetNormal() { return Normal.xyz; }
#endif	
};


struct KBloodTrail
{
#define BLOOD_TRAIL_PARTICLE_COUNT 12
	
	FLOAT4 Position;
	
#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const GVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetTimeCreated(f32 time) { Position.w = time; }

#else
	
	float3 GetPosition() { return Position.xyz; }
	float GetTimeCreated() { return Position.w; }
	
#endif
};

struct KBloodTrail_UnderWater
{
	FLOAT4 Position;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const GVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetTimeCreated(f32 time) { Position.w = time; }

#else

	float3 GetPosition() { return Position.xyz; }
	float GetTimeCreated() { return Position.w; }

#endif
};

struct KTorchFlame
{
#define TORCH_FLAME_PARTICLE_COUNT 64

	FLOAT4 Position;
	FLOAT4 Data;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const GVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	//void SetRandomSeed(f32 seed) { Position.w = seed; }

	void SetMainColor(FColor8 color) { memcpy(&Data.x, &color.Color, 4); }
	void SetOffColor(FColor8 color) { memcpy(&Data.y, &color.Color, 4); }

#else

	float3 GetPosition() { return Position.xyz; }
	//float GetRandomSeed() { return Position.w; }

	float4 GetMainColor()
	{
		float4 color;
		uint col = asuint(Data.x);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}

	float4 GetOffColor()
	{
		float4 color;
		uint col = asuint(Data.y);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}

#endif
};

struct KPowerupBrain
{
#ifdef __cplusplus
	MAT4 PrevModelMat;
	MAT4 CurrentModelMat;
	FLOAT4 Data;

	static class KModelObj* Model;
	static UPtr<KTexture2D> Texture;

	static void RenderBuffer(const KRenderBufferInfo& info);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info) { return true; }
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);

	void SetTeleported() { Data.x = 1; }
	void SetLastMoveRenderAlpha(f32 alpha) { Data.y = alpha; }
#endif
};

struct KAtomProjectile
{
#define ATOM_SHELL_COUNT 4
#define ATOM_SHELL_PARTICLE_COUNT 128

	FLOAT4 PrevPos;
	FLOAT4 CurrentPos;


#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const GVec3& pos)
	{
		PrevPos.x = pos.y;
		PrevPos.y = pos.z;
		PrevPos.z = pos.x;
	}

	void SetCurrentPosition(const GVec3& pos)
	{
		CurrentPos.x = pos.y;
		CurrentPos.y = pos.z;
		CurrentPos.z = pos.x;
	}

	void SetLastMoveAlpha(f32 alpha) { CurrentPos.w = alpha; }

#else

	float3 GetPosition(float alpha) { return lerp(PrevPos.xyz, CurrentPos.xyz, alpha); }
	
	float GetLastMoveAlpha() { return CurrentPos.w; }

#endif
};

struct KExplosion
{
#define EXPLODE_PARTICLE_COUNT 1024

	FLOAT4 Position;
	FLOAT4 Normal;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetNormal(const FVec3& norm)
	{
		Normal.x = norm.y;
		Normal.y = norm.z;
		Normal.z = norm.x;
	}

	void SetTimeCreated(f32 time) { Normal.w = time; }
	void SetRadius(f32 radius) { Position.w = radius; }

#else
	float GetTimeCreated() { return Normal.w; }
	float GetRadius() { return Position.w; }
	float3 GetPosition() { return Position.xyz; }
	float3 GetNormal() { return Normal.xyz; }
#endif	
};

struct KBlastExplosion
{
#define BLAST_EXPLODE_PARTICLE_COUNT 128

	FLOAT4 Position;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetTimeCreated(f32 time) { Position.w = time; }

#else
	float GetTimeCreated() { return Position.w; }
	float3 GetPosition() { return Position.xyz; }
#endif	
};

struct KRocketTrail
{
#define ROCKET_TRAIL_PARTICLE_COUNT 256

	FLOAT4 StartPosition;
	FLOAT4 Velocity;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetStartPosition(const FVec3& pos)
	{
		StartPosition.x = pos.y;
		StartPosition.y = pos.z;
		StartPosition.z = pos.x;
	}

	void SetVelocity(const FVec3& norm)
	{
		Velocity.x = norm.y;
		Velocity.y = norm.z;
		Velocity.z = norm.x;
	}

	void SetTimeCreated(f32 time) { Velocity.w = time; }
	void SetDeathTime(f32 time) { StartPosition.w = time; }

	// call this last
	void EnableRenderOffset()
	{
		Velocity.w = -Velocity.w;
	}

	// call this last too
	void EnableMyRocket()
	{
		if (StartPosition.w == 0) 
			StartPosition.w = -.01;
		else
			StartPosition.w = -StartPosition.w;
	}

#else
	float GetTimeCreated() { return abs(Velocity.w); }
	float GetDeathTime() { return StartPosition.w == -.01 ? 0 : abs(StartPosition.w); }
	float3 GetStartPosition() { return StartPosition.xyz; }
	float3 GetVelocity() { return Velocity.xyz; }
	bool UseRenderOffset() { return Velocity.w < 0; }
	bool IsMyRocket() { return StartPosition.w < 0; }
#endif	
};

struct KSmokeSheet
{
	FLOAT4 Position;
#ifdef __cplusplus

	static UPtr<KTexture2D> Texture;

	static void RenderBuffer(const KRenderBufferInfo& info);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info) { return true; }
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);

	void SetPosition(const GVec3& pos)
	{
		glm::vec3 v = pos.ToGLM();
		Position.x = v.x;
		Position.y = v.y;
		Position.z = v.z;
	}
#else
	float3 GetPosition()
	{
		return Position.xyz;
	}
#endif
};

struct KHealthCrystal
{
	MAT4 PrevModelMat;
	MAT4 CurrentModelMat;
	FLOAT4 Data;
#ifdef __cplusplus

	static class KModelObj* Model;

	static UPtr<class KTexture2D> Tex1;
	static UPtr<class KTexture2D> Tex2;

	static void RenderBuffer(const KRenderBufferInfo& info);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info) { return true; }
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);

	void SetTeleported() { Data.x = 1; }
	void SetLastMoveRenderAlpha(f32 alpha) { Data.y = alpha; }
	void SetColor(FColor8 color) { memcpy(&Data.z, &color.Color, 4); }
#else 
	float4 GetColor()
	{
		float4 color;
		uint col = asuint(Data.z);
		color.a = (col >> 24) / 255.f;
		color.r = ((col >> 16) & 0xFF) / 255.f;
		color.g = ((col >> 8) & 0xFF) / 255.f;
		color.b = (col & 0xFF) / 255.f;
		return color;
	}
#endif
};

struct KPortalTravel
{
#define PORTAL_TRAVEL_COUNT 256

	FLOAT4 Position;
	FLOAT4 HalfExtent;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPosition(const FVec3& pos)
	{
		Position.x = pos.y;
		Position.y = pos.z;
		Position.z = pos.x;
	}

	void SetHalfExtent(const FVec3& extent)
	{
		HalfExtent.x = extent.y;
		HalfExtent.y = extent.z;
		HalfExtent.z = extent.x;
	}

	void SetTimeCreated(f32 time) { Position.w = time; }
	void SetIsEntry(bool entry) { HalfExtent.w = entry; }

#else
	float GetTimeCreated() { return Position.w; }
	float3 GetPosition() { return Position.xyz; }
	float3 GetHalfExtent() { return HalfExtent.xyz; }
	bool IsEntry() { return HalfExtent.w > 0; }
#endif	
};

struct KBlasterParticle
{

#define BLASTER_PARTICLE_COUNT 128

	FLOAT4 PrevPosition;
	FLOAT4 CurrentPosition;

#ifdef __cplusplus

	static void RenderBuffer(const KRenderBufferInfo& info);
	static void CreateBuffers(KRenderBufferInfo& info, u32 typeSize, u32 elementCount);
	static bool ShouldBeUpdated(const KRenderBufferInfo& info);

	void SetPrevPosition(const FVec3& pos)
	{
		PrevPosition.x = pos.y;
		PrevPosition.y = pos.z;
		PrevPosition.z = pos.x;
	}

	void SetCurrentPosition(const FVec3& pos)
	{
		CurrentPosition.x = pos.y;
		CurrentPosition.y = pos.z;
		CurrentPosition.z = pos.x;
	}

	void SetTimeCreated(f32 time) { PrevPosition.w = time; }

#else
	float GetTimeCreated() { return abs(PrevPosition.w); }
	float3 GetPosition(float alpha) 
	{ 
		return lerp(PrevPosition.xyz, CurrentPosition.xyz, alpha);
	}
	float3 GetVelocity()
	{
		return (CurrentPosition.xyz - PrevPosition.xyz) / (1.f / 60.f);
	}
#endif	
};

struct KHealthParticles
{
	
};

#endif // BUFFER_SLOTS
#endif // server
