#pragma once

#include "kfglobal.h"
#include "engine/math/line_segment.h"
#include "engine/collision/hit_result.h"
#include "engine/math/aabb.h"
#include "bvh_node.h"

class KStaticBVH
{
	UPtr<KBvhNode> Head = nullptr;

public:

	KStaticBVH() = default;
	
	~KStaticBVH();

	class KBvhNode* GetHead() { return Head.get(); }

	void InitFromBrushes(TVector<class KCollisionBrush*> brushes);

	//bool TraceLine(const GLineSegment& line, GHitResult& hit, KTraceCondition condition);
	//bool TraceBox(const GLineSegment& line, const GVec3& extent, GHitResult& hit, KTraceCondition condition);
};