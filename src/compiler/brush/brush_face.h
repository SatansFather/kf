#pragma once

#include "engine/math/plane.h"
#include "engine/math/polygon.h"
#include "engine/math/triangle.h"
#include "engine/utility/kstring.h"
#include "brush_vertex.h"
#include "engine/math/aabb.h"
#include <mutex>

// DELETE
#if !_SERVER
#include "engine/math/glm.h"
#include "../light/luxel.h"
#endif


class KBrushFace
{
	friend class KMapBrush;
	friend class KMapCompiler;

#if !_SERVER
	friend class KRenderInterface; // temporary
#endif

//private:
public:

	DPlane Plane;
	DPolygon Poly;
	DVec3 Center;
	TVector<KBrushVertex> Vertices;
	TVector<KBrushVertex> OriginalVertices;
	f64 SurfaceArea;
	DBoundingBox Bounds;
	class KMapBrush* OwningBrush = nullptr;

	KBrushFace();
	~KBrushFace();

#if !_SERVER
	TVector<KBrushVertex> NewSmoothedVerts;
	TVector<DTriangle> Triangles;
	TVector<u32> Indices;
	KString TexName;
	DVec3 AxisU, AxisV;
	f64 TexOffsetX, TexOffsetY;
	f64 TexRotation;
	f64 TexScaleX, TexScaleY;
	f64 LightmapResolution = 4;
	u32 Content, Surface;
	TVector<i32> SmoothingGroups;
	i32 EntityID;
	u32 RadiosityPasses = 2;
	bool bSmoothed = false;
	class KTexture2D* Texture = nullptr;

	u32 BufferIndex = 0; // material parameters
	KString Material = "";
	bool bVolumeMaterial = false; // true if the face gets its material from the brush entity rather than faceattribs

	u32 BrushPlaneIndex = 0;

	bool bIsPortal = false;
	KBrushFace* AssociatedPortal = nullptr;
	TVector<class KBspNode*> LeafNodes;

	// face will have no role in the splitting process
	bool bNoSplit = false;

	bool bIsCollisionPass = false;

	u8 SurfaceIndex = 0;

	// flagged when this is determined to be a contact face
	// prevents re-testing it while not needing to remove it from vector during loop
	bool bPendingDelete = false;

	TVector<TVector<KLuxel>> Lightmap;
	bool bLightmapAllBlack = true;
	f64 StartX, EndX, StartY, EndY;
	class KFaceAttribs* FaceAttribs = nullptr;

	TVector<class KLightEntity*> ValidLights;

	u32 SetupIndexBuffer();

	void PrepareFace();
	//UPtr<class KSurface2D> LoadTextureAsSurface();
	void DetermineTextureCoordinates();
	bool ShouldHaveLight();

	DVec3 GetNormalAtPoint(DVec3 point);

	void AddSmoothingGroup(i32 group);

private:

	void PrepareLightingData();
	void SetupLightmapDimensions();
	void FindValidLights(TVector<KLightEntity*>& lights);
	void BuildDirectLighting();
	void RadiosityPass();

	void BuildDirectLighting_GPU();

	bool CalculateLightAtPoint(const DVec3& point, const DVec3& norm, DVec3& outcolor, class KLightEntity* light, DVec3* gi);
	DVec3 LightStrengthAtPoint(const DVec3& point, class KLightEntity* light) const;
	DVec3 MovePointTowardCenter(const DVec3& point, f64 distance = 0.5) const;
	void PadLightmap();

public:

	
	bool bOnNode = false; // move this!


	void DivideLargeFace(f64 max_dimension, TVector<std::unique_ptr<KBrushFace>>& out_faces);
	std::unique_ptr<KBrushFace> MergeWith(KBrushFace* other);
	//bool BoundingBoxesOverlap(KBrushFace* other, f64 epsilon = Epsilon<f64>()) const;
	//bool BoundingBoxesOverlap(class KMapBrush* other, f64 epsilon = Epsilon<f64>()) const;

	UPtr<KBrushFace> SplitByPlane(DPlane plane, f64 epsilon = .001);

	bool HasTransparency() const;

	TVector<KBrushVertex>& GetLitVertices();
	void InterpolateVertexLighting();
	const DPlane& GetPlane() const { return Plane; }
	const DPolygon& GetPolygon() const { return Poly; }
	const TVector<DTriangle>& GetTriangles() const { return Triangles; }
	const DVec3& GetCenter() const { return Center; }
	const TVector<KBrushVertex>& GetVertices() const { return Vertices; }
	const TVector<u32>& GetIndices() const { return Indices; }
	const KString& GetTexName() const { return TexName; }
	const DVec3& GetAxisU() const { return AxisU; }
	const DVec3& GetAxisV() const { return AxisV; }
	f64 GetTexOffsetX() const { return TexOffsetX; }
	f64 GetTexOffsetY() const { return TexOffsetY; }
	f64 GetTexRotation() const { return TexRotation; }
	f64 GetLightmapResolution() const { return LightmapResolution; }
	u32 GetContent() const { return Content; }
	u32 GetSurface() const { return Surface; }
	i32 GetEntityID() const { return EntityID; }
	KMapBrush* GetOwningBrush() const { return OwningBrush; }
	bool IsSmoothed() const { return bSmoothed; }
	KTexture2D* GetTexture() const { return Texture; }
	//DVec3 GetBoundingMin() { return BoundingMin; }
	//DVec3 GetBoundingMax() { return BoundingMax; }
	const DBoundingBox& GetBounds() { return Bounds; } 
	void SetMaterial(const KString& mat);
#endif
	void InitFromVertices();
	void SortVerticesCW();
	
#if _COMPILER
	TMap<KString, KString>& GetMaterialPropertyMap();
#endif

	KString GetMaterialProperty(const KString& propName);
};
