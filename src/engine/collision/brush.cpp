#include "brush.h"


KCollisionBrush::KCollisionBrush(KPendingCollisionBrush& data)
{
	Bounds = data.Bounds;
	CollisionChannels = data.CollisionChannels;
	CollisionPass = data.CollisionPass;
#if _COMPILER
	EntityID = data.EntityID;
#endif

	for (const GPlane& p : data.Planes)
		AddUniquePlane(p);

	AddBevelPlanes(data);
}

bool KCollisionBrush::HasPlane(const GPlane& plane, GFlt normEpsilon /*= .00001*/, GFlt distEpsilon /*= .01*/) const
{
	for (const GPlane& p : Planes)
	  if (p.Equals(plane, normEpsilon, distEpsilon))
		return true;

	return false;
}

i32 KCollisionBrush::GetPlaneIndex(const GPlane& plane) const
{
	for (i32 i = 0; i < Planes.size(); i++)
	  if (Planes[i].Equals(plane))
		return i;

	return -1;
}

GFlt KCollisionBrush::GetPlaneValue(const GPlane& p)
{
	// dot product between axial planes is -1, 0, or 1
	GVec3 d = p.Normal.z == 0 ? GVec3(1, 0, 0) : GVec3(0, 0, 1);
	GFlt val = acos(abs(p.Normal.Dot(d)));
	val = abs(val);

	// worst value is .707 for a 45 degree angle
	// result of this would be .5 for worst value
	val /= (PI<GFlt>() / 2.0);

	// worst value becomes 0
	// better planes will return a higher value
	val -= .5;
	return abs(val);
}

void KCollisionBrush::AddOrderedPlane(const GPlane& plane)
{
	if (!plane.IsAxial())
	{
		// stored planes are already sorted
		// new plane will sit before the first plane with a greater value
		GFlt a = GetPlaneValue(plane);
		for (u32 i = 0; i < Planes.size(); i++)
		{
			const GPlane& p = Planes[i];
			GFlt b = GetPlaneValue(p);
			if (b > a || p.IsAxial())
			{
				Planes.insert(Planes.begin() + i, plane);
				return;
			}
		}
	}

	Planes.push_back(plane);
}

void KCollisionBrush::AddUniquePlane(const GPlane& plane)
{
	if (!HasPlane(plane)) AddOrderedPlane(plane);
}

void KCollisionBrush::AddBevelPlanes(KPendingCollisionBrush& data)
{
	if (Planes.size() == 6)
	{
		// this brush is entirely axial
	}
	else
	{
		// add bevel planes
		for (const KPendingCollisionBrush::FaceData& face : data.Faces)
		{
			if (face.Plane.IsAxial()) continue;
			for (i32 edge = 0; edge < face.Poly.NumEdges(); edge++)
			{
				// find the face that share this line
				// only keep it if the angle between them is acute
				bool lineMatch = false;
				const GLineSegment& line = face.Poly.Edge(edge);
				for (const KPendingCollisionBrush::FaceData& face2 : data.Faces)
				{
					if (lineMatch) break;
					if (face.Plane == face2.Plane) continue;
					for (i32 edge2 = 0; edge2 < face2.Poly.NumEdges(); edge2++)
					{
						const GLineSegment& line2 = face2.Poly.Edge(edge2);
						if (line2.Equals(line))
						{	// found shared face
							if ((face.Plane.Normal | face2.Plane.Normal) < 0)
							{
								lineMatch = true;
								break;
							}
						}
					}
				}

				if (!lineMatch) continue;

				GVec3 lineDir = line.b - line.a;
				if (lineDir.Normalize() < .5) continue;
				i32 axis;
				for (axis = 0; axis < 3; axis++)
				{
					if (abs(lineDir[axis] - 1) < .00001)
					{
						lineDir = 0;
						lineDir[axis] = 1;
						break;
					}
					if (abs(lineDir[axis] - -1) < .00001)
					{
						lineDir = 0;
						lineDir[axis] = -1;
						break;
					}
				}

				for (axis = 0; axis < 3; axis++)
				  //if (lineDir[axis] == -1 || lineDir[axis] == 1)
				  if (abs(lineDir[axis]) > .9999)
					break;

				// skip axial edges
				if (axis != 3) continue;

				for (axis = 0; axis < 3; axis++)
				{
					for (i32 dir = -1; dir <= 1; dir += 2)
					{
						GVec3 axisDir;
						axisDir[axis] = dir;
						GVec3 normal = lineDir ^ axisDir;
						if (normal.Normalize() < .5) continue;

						GPlane p(line.a, normal);
						if (HasPlane(p)) continue;

						i32 k;
						for (k = 0; k < data.Faces.size(); k++)
						{
							// if all brush vertices are behind this plane, it is a proper edge bevel
							i32 point;
							for (point = 0; point < data.Faces[k].Poly.Points.size(); point++)
							{
								if (p.ClassifyPoint(data.Faces[k].Poly.Points[point], .1) == EPointSide::Front)
									break;
							}
							if (point != data.Faces[k].Poly.Points.size()) break;
						}
						if (k != data.Faces.size()) continue;

						//Planes.push_back(p);
						//Planes.insert(Planes.begin(), p);
						AddUniquePlane(p);
					}
				}
			}
		}
	}
}

TVector<GPolygon> KCollisionBrush::CalculatePolygons()
{
	const auto pointInsideBrush = [this](const GVec3& p) -> bool
	{
		for (GPlane& plane : Planes)
		  if ((plane.Normal | (p - plane.PointOnPlane())) > .01)
			return false;

		return true;
	};
	TVector<GPolygon> polys;
	polys.resize(Planes.size());
	i32 planeCount = i32(Planes.size());
	TVector<GVec3> intersections;
	for (i32 i = 0; i < planeCount - 2; i++)
	{
		for (i32 j = i; j < planeCount - 1; j++)
		{
			for (i32 k = j; k < planeCount; k++)
			{
				if (i != j && i != k && j != k)
				{
					GVec3 intersection = GVec3(0, 0, 0);
					if (Planes[i].Intersects(Planes[j], Planes[k], &intersection))
					{
						if (pointInsideBrush(intersection))
						{
							if (!VectorContains(intersections, intersection))
							{
								intersections.push_back(intersection);
							}

							// only add unique vertices to polys
							auto AddUnique = [&](GPolygon& poly, GVec3& v)
							{
								for (GVec3& p : poly.Points)
								  if (p.Equals(v))
									return;

								poly.Points.push_back(v);
							};

							AddUnique(polys[i], intersection);
							AddUnique(polys[j], intersection);
							AddUnique(polys[k], intersection);
						}
					}
				}
			}
		}
	}

	Center = 0;
	for (const GVec3& p : intersections)
		Center += p;

	Center /= intersections.size();

	return polys;
}

bool KPendingCollisionBrush::HasPlane(const GPlane& plane)
{
	for (GPlane& p : Planes)
	  if (p.Equals(plane))
		return true;

	return false;
}
