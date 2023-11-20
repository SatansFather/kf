#include "brush_vertex.h"

void KBrushVertex::AddSmoothedNormal(const DVec3& norm)
{
	if (norm.Equals(Normal, .001)) return;

	for (const DVec3& n : PendingSmoothNormals)
		if (n.Equals(norm, .001)) return;

	PendingSmoothNormals.push_back(norm);
}
