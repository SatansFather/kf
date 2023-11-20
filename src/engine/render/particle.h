/*
#pragma once

#if !_SERVER

#include "kfglobal.h"
#include "engine/math/glm.h"
#include "engine/render/bufferslots.h"

enum class EParticleSystemUpdateRate : u8
{
	UpdateOnce,			// created with level, set and forget
	UpdateSometimes,	// spawn sporadically (tele effects, explosions, etc)
	UpdatePerFrame,		// attached to moving objects
};

class KParticleSystem
{
	// how often the buffer needs to be updated with new data
	// whether by new creation or update of existing
	const EParticleSystemUpdateRate UpdateRate;
	const u32 TypeSize;
	const u32 BufferCount;

	UPtr<u8[]> Buffer;
	UPtr<u8[]> AltBuffer;

	/ *static UPtr<u8[]> OneTimeBuffer;
	static UPtr<u8[]> PerFrameBuffer;
	UPtr<u8[]> SporadicBuffer;* /

public:

	//void ClearBuffer();
	//static void RemoveAllParticles();

	KParticleSystem(EParticleSystemUpdateRate updateRate, u32 typeSize, u32 bufferCount);

	EParticleSystemUpdateRate GetUdpateRate() const { return UpdateRate; }
	virtual u32 GetTypeSize() const = 0;
};

/ *
template <typename ParticleType>
class KParticleSystem : public KParticleSystemBase
{
	// template only exists so this buffer can exist
	static UPtr<ParticleType[]> Buffer;

public:
	u32 GetTypeSize() const override { return sizeof(ParticleType); }
};* /

struct KParticleSystemCB
{
	u32 StartOffset;
	f32 pad1, pad2, pad3;
};

class KParticleBuffer
{
public:
	static UPtr<KBaseParticle[]> MasterBuffer;
	static UPtr<class KGpuBuffer> MasterGpuBuffer;
	static UPtr<class KVertexBuffer> ParticleIndexBuffer;

	// individual particle types will hold a position in MasterBuffer
	KBaseParticle* Data = nullptr;
	u32 MaxParticles = 0;
	u32 StartOffset = 0;
	u32 EndOffset = 0;
	f32 ParticleLifetime = 1;

public:
	u32 ActiveCount = 0;

	static UPtr<class KParticleBuffer> TestBuffer;

public:

	~KParticleBuffer();

	u32 GetBufferSize() const { return sizeof(KBaseParticle) * MaxParticles; }
	u32 GetStartOffset() const { return StartOffset; }

	KBaseParticle* GetData() { return Data; }
	KBaseParticle* AddParticle(const KBaseParticle& particle);
	//void AddParticle(const glm::vec4& pos, const glm::vec4& color = glm::vec4(1, 1, 1, 1), const glm::vec4& data = glm::vec4(0, 0, 0, 0));

	static void AllocateBuffers();
	static void DrawParticles();
};

#endif*/