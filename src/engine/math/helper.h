#pragma once
#pragma once

#include "math.h"
#include "glm.h"

// builds an orthographic matrix over a face with the minimum width and height
// vertex array assumes clockwise winding
template <typename T>
void GetViewOverVertices(TVector<KVec3<T>>& verts, const glm::mat4& model, glm::mat4& outMVP, f32& w, f32& h, TVector<glm::vec2>& outUVs)
{
	using Polygon = KPolygon<T>;
	using Vec3 = KVec3<T>;
	using Plane = KPlane<T>;
	using namespace glm;

	K_ASSERT(verts.size() > 2, "cannot form a polygon with less than 3 vertices");

	Polygon poly;
	TVector<Vec3> edgepoints(2);
	T longestedge = 0;
	for (u32 i = 0; i < verts.size(); i++)
	{
		Vec3& v = verts[i];
		poly.Points.push_back(v);

		if (i > 0)
		{
			T dist = v.Distance(verts[i - 1]);
			if (dist > longestedge)
			{
				longestedge = dist;
				edgepoints[0] = verts[i - 1];
				edgepoints[1] = v;
			}
		}
	}

	K_ASSERT(longestedge > 0, "longest edge of polygon was 0");

	Vec3 basisW = edgepoints[1] - edgepoints[0];
	basisW.Normalize();

	Vec3 normal = (verts[0] - verts[1]) ^ (verts[2] - verts[1]);
	normal.Normalize();

	Vec3 basisH = basisW ^ normal;
	basisH.Normalize();

	Vec3 wa = poly.ExtremePoint(-basisW);
	Vec3 wb = poly.ExtremePoint(basisW);
	Vec3 ha = poly.ExtremePoint(-basisH);
	Vec3 hb = poly.ExtremePoint(basisH);

	Plane wpmin(wa, basisW);
	Plane wpmax(wb, basisW);
	Plane hpmin(ha, basisH);
	Plane hpmax(hb, basisH);

	w = wpmin.Distance(wb);
	h = hpmin.Distance(hb);

	Vec3 bottomleft = wa + (basisH * -hpmin.Distance(wa));
	Vec3 topright = wb + (basisH * hpmax.Distance(wb));
	Vec3 center = (bottomleft + topright) / 2.0;
	
	Vec3 up = abs(normal.z) == 1 ? Vec3(0, 1, 0) : Vec3(0, 0, 1);
	Vec3 right = (up ^ normal).GetNormalized();

	// clamp dot to -1,1 to avoid numbers like 1.0000000000002332 causing acos to return nan
	f64 d = right | basisW;
	f64 angle = acos( std::clamp(d, -1.0, 1.0) );
	d = normal | (right ^ basisW);

	bool negAngle = false;
	if (d != 0)
	{
		negAngle = true;
		angle *= sign( std::clamp(d, -1.0, 1.0) );
	}

	// switch to glm types //

	vec3 norm = normal.ToGLM();
	vec3 cen = center.ToGLM();
	vec3 pos = cen + (norm * 48.f);

	mat4 view = lookAt(pos, pos + norm, vec3(up.ToGLM()));
	view = translate(view, cen);
	view = rotate(view, f32(-angle), norm);
	view = translate(view, -cen);

	mat4 proj = ortho(-w/2, w/2, h/2, -h/2, 1.f, 1000.f);

	outMVP = transpose(proj * view * model);

	// get UVs for each vertex
	for (Vec3& vert : verts)
	{
		f32 u = wpmax.Distance(vert) / w;
		f32 v = hpmax.Distance(vert) / h;

		outUVs.push_back(glm::vec2(u, v));
	}
}

void GetViewOverVertices(TVector<glm::vec3>& verts, const glm::mat4& model, glm::mat4& outMVP, f32& w, f32& h, TVector<glm::vec2>& outUVs)
{
	TVector<FVec3> v(verts.size());
	for (auto& vert : verts) { v.push_back(FVec3::FromGLM(vert)); }
	GetViewOverVertices<f32>(v, model, outMVP, w, h, outUVs);
}