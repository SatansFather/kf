#include "bsp_grid.h"
#include "../brush/brush_face.h"

KBspGrid::KBspGrid(TVector<UPtr<class KBrushFace>>& faces, u32 dimension /*= 256*/)
	: Dimension(dimension)
{
	DBoundingBox bounds;
	for (UPtr<KBrushFace>& face : faces)
	  for (const KBrushVertex& v : face->GetVertices())
		bounds.Update(v.Point);

	DVec3 gridSizes = (bounds.Max - bounds.Min) / dimension;
	TVector<TVector<TVector<TVector<UPtr<KBrushFace>>>>> faceGrid;
	faceGrid.resize(u32(gridSizes.x));
	for (auto& vecY : faceGrid)
	{
		vecY.resize(u32(gridSizes.y));
		for (auto& vecZ : vecY)
			vecZ.resize(u32(gridSizes.z));
	}	

	// create grid planes
	TVector<DPlane> gridPlanes;
	u32 xIndex = 0;
	u32 yIndex = 0;
	u32 zIndex = 0;
	for (i32 i = 0; i < 3; i++)
	{
		DVec3 norm;
		norm[i] = 1;
		DPlane plane;
		plane.Normal = norm;
		for (f64 comp = bounds.Min[i]; comp < bounds.Max[i] + dimension; comp += dimension)
		{
			plane.D = comp;
			gridPlanes.push_back(plane);
		}

		if (i == 0) yIndex = gridPlanes.size();
		if (i == 1) zIndex = gridPlanes.size();
	}

	// split faces along grid planes
	for (const DPlane& p : gridPlanes)
	{
		for (UPtr<KBrushFace>& face : faces)
		{
			EPolyClassification polyclass = face->Poly.ClassifyToPlane(p, .001);
			if (polyclass == EPolyClassification::Spanning)
			{
				UPtr<KBrushFace> split = face->SplitByPlane(p);
				faces.push_back(std::move(split));
			}
			else if (polyclass == EPolyClassification::Coplanar)
			{
				
			}
		}
	}

	for (UPtr<KBrushFace>& face : faces)
	{
		// determine cell index for each face
		DVec3 cell = face->Center - bounds.Min;
		cell /= Dimension;

		faceGrid[u32(cell.x)][u32(cell.y)][u32(cell.z)].push_back(std::move(face));
	}
	faces.clear();

	for (u32 x = 0; x < faceGrid.size(); x++)
	{
		for (u32 y = 0; y < faceGrid[x].size(); y++)
		{
			for (u32 z = 0; z < faceGrid[x][y].size(); z++)
			{
				

				// TODO create surfaces from grid planes, slice each one from both sides of plane, add to both trees
				KBspTree tree(faceGrid[x][y][z], false);
			}	
		}
	}
	faceGrid.clear();
}
