#include "bvh_node.h"
#include "../brush.h"

UPtr<KBvhNode> KBvhNode::CreateLeaf(KCollisionBrush* collision)
{
	UPtr<KBvhNode> node = std::make_unique<KBvhNode>();
	node->LeafCollision = collision;
	node->bIsLeaf = true;
	node->Bounds = collision->Bounds;
	return node;
}

