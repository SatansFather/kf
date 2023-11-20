#pragma once

#include "kfglobal.h"
#include "engine/math/aabb.h"

class KBvhNode
{
	friend class KStaticBVH;

	bool bIsLeaf = false;

	UPtr<KBvhNode> ChildA = nullptr;
	UPtr<KBvhNode> ChildB = nullptr;

	GBoundingBox Bounds;

	class KCollisionBrush* LeafCollision = nullptr;

	static UPtr<KBvhNode> CreateLeaf(class KCollisionBrush* collision);

public:

	bool IsLeaf() { return bIsLeaf; }
	
	KBvhNode* GetA() { return ChildA.get(); }
	KBvhNode* GetB() { return ChildB.get(); }
	const GBoundingBox& GetBounds() { return Bounds; }
	KCollisionBrush* GetLeafCollision() { return LeafCollision; }
};