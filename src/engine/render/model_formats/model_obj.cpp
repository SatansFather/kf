#if !_SERVER


#include "model_obj.h"
#include "tiny_obj_loader.h"
#include "engine/global/paths.h"
#include "engine/render/interface/render_interface.h"
#include "engine/render/interface/buffers/vertex_buffer.h"
#include "engine/render/vertex_types.h"

#include "../../kwad/kwad_chunk_obj.h"
#include "../../kwad/kwad_file.h"
#include "../../game_instance.h"
#include "../../kwad/kwad_reader.h"

#define TINYOBJLOADER_IMPLEMENTATION

using namespace tinyobj;

KModelObj::~KModelObj() {}

void KModelObj::LoadFromFile(const KString& name)
{

	KString file = RES_DIR + "model/" + name + ".obj";

	ObjReaderConfig cfg;
	cfg.triangulate = true;
	cfg.mtl_search_path = RES_DIR + "model";
	ObjReader reader;

#if _PACK
	KGameInstance::Get().MainWadFile.AddObjModel(name);
	return;
#elif _DEV
	if (!reader.ParseFromFile(file, cfg))
	{
		K_ASSERT(reader.Error().empty(), KString("obj read error: " + reader.Error()));
		return;
	}
#else

	KWadReader<KWadChunk_Obj> r("kfdata", name);
	KWadChunk_Obj* chunk = r.GetChunk();

	if (!reader.ParseFromString(chunk->Obj, chunk->Mtl, cfg))
	{
		K_ASSERT(reader.Error().empty(), KString("obj read error: " + reader.Error()));
		return;
	}
#endif

	if (!reader.Warning().empty())
		LOG(reader.Warning(), 1, 1, 0);

	const attrib_t& attrib = reader.GetAttrib();
	const TVector<shape_t>& shapes = reader.GetShapes();
	const TVector<material_t>& materials = reader.GetMaterials();

	TVector<KMeshVertex> vertices;

	for (size_t s = 0; s < shapes.size(); s++) 
	{	
		// faces
		size_t indexOffset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
		{
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// vertices
			for (size_t v = 0; v < fv; v++) 
			{
				index_t idx = shapes[s].mesh.indices[indexOffset + v];
				f32 vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				f32 vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				f32 vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				f32 tx = 0, ty = 0, nx = 0, ny = 0, nz = 0;

				// negative = no texcoord data
				if (idx.texcoord_index >= 0) 
				{
					tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					ty = 1 - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// negative = no normal data
				if (idx.normal_index >= 0) 
				{
					nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				vertices.push_back( { vx, vy, vz, tx, ty, nx, ny, nz } );
			}
			indexOffset += fv;


			// per-face material

			//shapes[s].mesh.material_ids[f];
		}
	}
	
	VertexCount = vertices.size();
	VertexBuffer = GetRenderInterface()->CreateVertexBufferStatic(vertices.data(), sizeof(KMeshVertex), vertices.size());
}

void KModelObj::Draw()
{
	KRenderInterface* iface = GetRenderInterface();
	iface->BindVertexBuffer(VertexBuffer.get());
	iface->Draw(VertexCount, 0);
}

#endif
