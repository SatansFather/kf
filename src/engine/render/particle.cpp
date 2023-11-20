/*
#if !_SERVER

#include "particle.h"
#include "bufferslots.h"
#include "interface/render_interface.h"
#include "engine/utility/random.h"
#include "interface/rasterizer.h"
#include "interface/buffers/vertex_buffer.h"
#include "interface/buffers/gpu_buffer.h"

// DELETE
#include "engine/system/terminal/terminal.h"

UPtr<KBaseParticle[]> KParticleBuffer::MasterBuffer;
UPtr<KGpuBuffer> KParticleBuffer::MasterGpuBuffer;
UPtr<KVertexBuffer> KParticleBuffer::ParticleIndexBuffer;
UPtr<KParticleBuffer> KParticleBuffer::TestBuffer;
KParticleBuffer::~KParticleBuffer() {}

KBaseParticle* KParticleBuffer::AddParticle(const KBaseParticle& particle)
{
	KBaseParticle* bufferEntry = Data + EndOffset;
	*bufferEntry = particle;
#if _COMPILER
	bufferEntry->SetTimeCreated(0);
#else
	bufferEntry->SetTimeCreated(MatchLifeTime());
#endif
	bufferEntry->SetTimeOffset(RandFloat(0, 3));
	EndOffset = (EndOffset + 1) % MaxParticles;
	ActiveCount++;
	if (ActiveCount > MaxParticles) ActiveCount = MaxParticles;
	return bufferEntry;
}

void KParticleBuffer::AllocateBuffers()
{
	if (MasterBuffer) return;
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	// find total size of all needed buffers
	u32 total = 0;

	//total += PB_TEST;
	//MasterBuffer = std::make_unique<KBaseParticle[]>(total);
	//MasterGpuBuffer = iface->CreateStructuredBufferDynamic(sizeof(KBaseParticle), 0, EShaderStage::Vertex, total);

	MasterGpuBuffer = iface->CreateStructuredBufferDynamic(sizeof(KProjectileRender), 0, EShaderStage::Vertex, 2048);

	// create index buffer
	u32 ind[6] = { 0, 2, 1, 2, 3, 1 };
	TVector<u32> indices;
	total = 2048;
	indices.resize(6 * total);
	for (u32 i = 0; i < total * 6; i++)
	{
		u32 index = i % 6;
		u32 particle = i / 6;
		indices[i] = ind[index] + particle * 4;
	}
	ParticleIndexBuffer = iface->CreateVertexBufferStatic(nullptr, 0, 0, indices);

	// reset total and use it to track buffer offset
	/ *KBaseParticle* master = MasterBuffer.get();
	memset(master, 0, total);
	total = 0;
	const auto createBuffer = [&total, master, iface](UPtr<KParticleBuffer>& buff, u32 count, f32 lifeTime) -> void
	{
		buff = std::make_unique<KParticleBuffer>();
		buff->Data = master + total;
		buff->MaxParticles = count;
		buff->ParticleLifetime = lifeTime;
		total += count;
	};

	createBuffer(TestBuffer, PB_TEST, 1);* /
}

bool corn = false;
void KParticleBuffer::DrawParticles()
{
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	//u8* data = iface->RenderBufferPointers[&typeid(KProjectileRender)].Data.get();
	//u32 active = iface->RenderBufferPointers[&typeid(KProjectileRender)].ActiveCount;
	//if (active == 0 || !data) return;

	//iface->UpdateBuffer(MasterGpuBuffer.get(), data);

	iface->BindBuffer(MasterGpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(ParticleIndexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.Particle.get());

	//iface->DrawIndexed(iface->RenderBufferPointers[&typeid(KProjectileRender)].ActiveCount * 6, 0);

	return;

	if (true || !corn)
	{
		// remove old particles
		/ *KBaseParticle* data = TestBuffer->GetData();
		for (TestBuffer->StartOffset; 
			iface->GetTotalRenderTime() - (data + TestBuffer->StartOffset)->GetTimeCreated() > 2		
			&& TestBuffer->ActiveCount > 0;
			TestBuffer->StartOffset = (TestBuffer->StartOffset + 1) % PB_TEST)
		{
			memset(data + TestBuffer->StartOffset, 0, sizeof(KBaseParticle));
			TestBuffer->ActiveCount--;
		}

		iface->BindParticleSystem(TestBuffer.get());* /


		// TODO buffers that only update once should not be a part of the master buffer
		iface->UpdateBuffer(MasterGpuBuffer.get(), MasterBuffer.get());
		corn = true;
	}
	iface->BindBuffer(MasterGpuBuffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(ParticleIndexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.Particle.get());

	iface->DrawIndexed(TestBuffer->ActiveCount * 6, 0);
}


KParticleSystem::KParticleSystem(EParticleSystemUpdateRate updateRate, u32 typeSize, u32 bufferCount)
	: UpdateRate(updateRate), TypeSize(typeSize), BufferCount(bufferCount)
{
	//Buffer = std::make_unique<
}

#endif
*/
