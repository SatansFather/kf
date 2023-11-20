#if !_SERVER

#include "bufferslots.h"
#include "interface/render_interface.h"
#include "interface/texture2d.h"
#include "engine/render/precipitation.h"
#include "interface/rasterizer.h"

KPrecipitationBuffer::~KPrecipitationBuffer() {}

void KPrecipitationBuffer::Init(const KPrecipitationVolume& p)
{
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	TVector<TVector<GVec3>> spawnPoints;
	TVector<TVector<GVec3>> stopPoints;
	p.GetSpawnPoints(spawnPoints);
	p.GetStopPoints(stopPoints);

	if (spawnPoints.size() == 0) return;
	K_ASSERT(stopPoints.size() == spawnPoints.size()
		&& stopPoints[0].size() == spawnPoints[0].size(),
		"precipitation start and stop point sizes didnt match");

	u32 w = spawnPoints.size();
	u32 h = spawnPoints[0].size();

	PrecipMin = p.GetBoundsMin().ToGLM4();
	PrecipMax = p.GetBoundsMax().ToGLM4();
	PrecipMin.w = w;
	PrecipMax.w = h;

	PrecipData.x = p.GetDensity();
	PrecipData.y = p.GetSpeed();
	PrecipData.z = p.GetSpacing();
	PrecipData.w = p.GetSway();

	struct BufferUpdate
	{
		glm::vec4 min, max, data; 
	};

	Buffer = iface->CreateConstantBufferStatic(sizeof(BufferUpdate), CB_PRECIP, EShaderStage::Vertex);
	BufferUpdate update { PrecipMin, PrecipMax, PrecipData };
	Buffer->Update(&update);

	UPtr<f32[]> zArray = std::make_unique<f32[]>(w * h);
	u32 count = 0;
		for (u32 y = 0; y < h; y++)
	{
	for (u32 x = 0; x < w; x++)
		{
			f32* point = zArray.get() + count;
			*point = stopPoints[x][y].z;
			count++;
		}
	}
	StopPoints = iface->CreateStructuredBufferStatic(sizeof(f32), 0, EShaderStage::Vertex, w * h);
	iface->UpdateBuffer(StopPoints.get(), zArray.get());
	IndexBuffer = iface->CreateParticleIndexBuffer(w * h);

	bInitialized = true;
}

void KPrecipitationBuffer::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	if (!iface) return;

	iface->EnableTransparency();
	iface->SetDepthBufferReadOnly(true);;
	iface->BindBuffer(Buffer.get());
	iface->GetRasterizer()->SetFillMode(EFillMode::Solid);
	iface->GetRasterizer()->SetCullMode(ECullMode::CullNone);
	iface->BindVertexBuffer(IndexBuffer.get());
	iface->BindShaderProgram(iface->Shaders.Snow.get());
	//iface->BindTexture2D(StopPoints.get(), 1, EShaderStage::Vertex);
	iface->BindBuffer(StopPoints.get());

	u32 particleCount = PrecipMin.w * PrecipMax.w;
	iface->DrawIndexed(particleCount * 6, 0);

	iface->DisableTransparency();
}

#endif
