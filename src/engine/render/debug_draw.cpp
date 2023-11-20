#if !_SERVER

#include "debug_draw.h"
#include "interface/render_interface.h"
#include "interface/buffers/gpu_buffer.h"
#include "interface/rasterizer.h"
#include "interface/buffers/vertex_buffer.h"

UPtr<KGpuBuffer> KDebugLine::DebugLineBuffer;
UPtr<KVertexBuffer> KDebugLine::DebugIndexBuffer;
KDebugLineInfo KDebugLine::LineData[LINE_BUFFER_SIZE];
u32 KDebugLine::LineCount = 0;

void KDebugLine::AddLine(const KDebugLineInfo& info)
{
	if (!DebugLineBuffer)
	{
		DebugLineBuffer = GetRenderInterface()->CreateStructuredBufferDynamic
						(sizeof(KDebugLineInfo), 0, EShaderStage::Vertex, LINE_BUFFER_SIZE);
		
		/*u32 ind[6] = { 0, 1, 2, 1, 2, 3 };
		TVector<u32> indices;
		indices.resize(6 * 4 * LINE_BUFFER_SIZE);
		for (u32 i = 0; i < LINE_BUFFER_SIZE * 6 * 4; i++)
		{
			u32 a = i / 6;
			u32 b = i % 6;
			indices[i] = ind[b] + a * 6;
		}*/

		u32 ind[6] = { 0, 2, 1, 2, 3, 1 };
		TVector<u32> indices;
		indices.resize(6 * LINE_BUFFER_SIZE);
		for (u32 i = 0; i < LINE_BUFFER_SIZE * 6; i++)
		{
			u32 index = i % 6;
			u32 particle = i / 6;
			indices[i] = ind[index] + particle * 4;
		}

		DebugIndexBuffer = GetRenderInterface()->CreateVertexBufferStatic(nullptr, 0, 0, indices);
	}

	LineData[LineCount] = info;
	LineCount++;
	GetRenderInterface()->UpdateBuffer(DebugLineBuffer.get(), &LineData);
}

void KDebugLine::AddLine(const GVec3& start, const GVec3& target, f32 r /*= 1*/, f32 g /*= 0*/, f32 b /*= 1*/, f32 thickness /*= 2*/)
{
	//AddLine( { start.ToGLM(), target.ToGLM(), { r, g, b }, thickness } );
	AddLine( { glm::vec4(start.ToGLM(), thickness), { r, g, b, 1.f } } );
}

void KDebugLine::DrawLines()
{
	if (KRenderInterface* i = GetRenderInterface())
	{
		i->GetRasterizer()->SetFillMode(EFillMode::Solid);
		i->GetRasterizer()->SetCullMode(ECullMode::CullNone);
		i->BindBuffer(DebugLineBuffer.get());
		i->BindVertexBuffer(DebugIndexBuffer.get());
		//i->BindShaderProgram(i->Shaders.DebugLine.get());
		//i->DrawIndexed(6 * 4 * LineCount, 0);
		//i->BindShaderProgram(i->Shaders.Particle.get());
		i->DrawIndexed(6 * LineCount, 0);
	}
}

#endif
