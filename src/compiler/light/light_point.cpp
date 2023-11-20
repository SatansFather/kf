#include "light_point.h"
#include "../entity.h"
#include "engine/system/terminal/terminal.h"

UPtr<KLightEntity_Point> KLightEntity_Point::Create(class KMapEntity* ent)
{
	UPtr<KLightEntity_Point> light = std::make_unique<KLightEntity_Point>();
	
	//ParseCommonLightProperies();













	KString origin = ent->GetProperty("origin");
	{
		TVector<f32> o = origin.ToFloatArray();
		if (o.size() == 3)
		{
			DVec3 pos(o[0], o[1], o[2]);
			light->Position = pos;
		}
		else
		{
			SYSLOG_ERROR(KString("Discarding point light entity with invalid origin \'" + origin + "\'"));
			light.reset();
			return light;
		}
	}

	KString color = ent->GetProperty("color");
	if (!color.IsEmpty())
	{
		TVector<f32> c = color.ToFloatArray();
		if (c.size() == 3)
		{
			light->Color.x = c[0];
			light->Color.y = c[1];
			light->Color.z = c[2];
		}
		else
		{
			SYSLOG_ERROR(KString("Discarding point light entity at (" + origin + ") with invalid color \'" + color + "\'"));
			light.reset();
			return light;
		}
	}
	//light->Color *= 5;

	KString gi_scale = ent->GetProperty("gi_scale");
	if (!gi_scale.IsEmpty())
	{
		TVector<f32> gi = gi_scale.ToFloatArray();
		if (gi.size() == 3)
		{
			light->GIScale.x = gi[0];
			light->GIScale.y = gi[1];
			light->GIScale.z = gi[2];
		}
		else
		{
			SYSLOG_WARNING(KString("Point light entity at (" + origin + ") with invalid gi_scale \'" + gi_scale + "\' - Defaulting to (1 1 1)"));
		}
	}

	KString brightness = ent->GetProperty("brightness");
	if (!brightness.IsEmpty())
	{
		f32 bright = -1;
		if (brightness.ToFloatSafe(bright))
		{
			if (bright < 0) bright = 0;
			light->Color *= bright;
		}
		else
		{
			SYSLOG_WARNING(KString("Point light entity at (" + origin + ") with invalid brightness \'" + brightness + "\' - Defaulting to 1"));
		}
	}

	KString shadows = ent->GetProperty("cast_shadows");
	if (!shadows.IsEmpty())
	{
		f32 cast = 1;
		if (shadows.ToFloatSafe(cast))
		{
			light->bCastShadows = cast > 0;
		}
	}	














	KString falloff = ent->GetProperty("falloff_exponent");
	if (!falloff.IsEmpty())
	{
		f32 fall = -1;
		if (falloff.ToFloatSafe(fall))
		{
			light->Falloff = fall;
		}
		else
		{
			SYSLOG_WARNING(KString("Point light entity at (" + origin + ") with invalid falloff_exponent \'" + falloff + "\' - Defaulting to 1"));
		}
	}

	KString focus = ent->GetProperty("focus");
	if (!focus.IsEmpty())
	{
		f32 f = 1;
		if (focus.ToFloatSafe(f))
		{
			light->Focus = f;
		}
		else
		{
			SYSLOG_WARNING(KString("Point light entity at (" + origin + ") with invalid focus \'" + focus + "\' - Defaulting to 1"));
		}
	}

	KString range = ent->GetProperty("range");
	if (!range.IsEmpty())
	{
		f32 r = 1;
		if (range.ToFloatSafe(r))
		{
			light->Range = r;
		}
		else
		{
			SYSLOG_WARNING(KString("Point light entity at (" + origin + ") with invalid range \'" + focus + "\' - Defaulting to 256"));
		}
	}

	return light;
}

bool KLightEntity_Point::PolyInRange(const DPolygon& poly) const
{
	return poly.Distance(Position) <= Range;
}

bool KLightEntity_Point::PointInRange(const DVec3& point) const
{
	return point.DistanceSq(GetPosition()) < Range * Range;
}

bool KLightEntity_Point::PolyBoundsInLightBouds(const DPolygon& poly) const
{
	// create bounding box from light
	DBoundingBox lightBounds;
	lightBounds.Update(GetPosition() + Range);
	lightBounds.Update(GetPosition() - Range);

	DBoundingBox polyBounds;
	for (const DVec3& v : poly.Points)
	  polyBounds.Update(v);

	return polyBounds.Overlaps(lightBounds);
}

bool KLightEntity_Point::CanHitPlane(const DPlane& plane) const
{
	return ((plane.PointOnPlane() - Position) | plane.Normal) < 0;
}

DVec3 KLightEntity_Point::FromPoint(const DVec3& point) const
{
	return Position - point;
}

f64 KLightEntity_Point::Strength(f64 diffuseFactor, f64 dist) const
{
	f64 falloff = 1 - pow((dist / Range), Falloff);
	diffuseFactor = pow(pow(diffuseFactor, 1.f / (Range / 256.f)), Focus);
	return diffuseFactor * falloff;
}

DVec3 KLightEntity_Point::GetPosition(DVec3 point) const
{
	return Position;
}
