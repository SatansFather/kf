#include "static_bvh.h"
#include "../brush.h"
#include "bvh_node.h"
#include "engine/system/time.h"

// DELETE
#include "../trace.h"
#include "engine/system/terminal/terminal.h"
#include "mutex"

#if !_SERVER && !_COMPILER
#include "../../console/engine_console.h"
#endif

KStaticBVH::~KStaticBVH() {}

void KStaticBVH::InitFromBrushes(TVector<class KCollisionBrush*> brushes)
{
	// create a leaf for each brush
	// start them as ungrouped and move them out as they get a partner
	TVector<UPtr<KBvhNode>> ungrouped;
	for (KCollisionBrush* brush : brushes) 
		ungrouped.push_back(std::move(KBvhNode::CreateLeaf(brush)));

	// sort smallest to largest
	std::sort
	(
		ungrouped.begin(),
		ungrouped.end(),
		[&](const UPtr<KBvhNode>& a, const UPtr<KBvhNode>& b) -> bool
			{ return a->Bounds.GetArea() < b->Bounds.GetArea(); }
	);

	TVector<KBvhNode*> grouped;
	for (i32 i = 0; i < ungrouped.size(); i++)
	{
		KBvhNode* node = ungrouped[i].get();

		KBvhNode* best = nullptr;
		GBoundingBox bestbox;
		bestbox.Reset();
		i32 bestindex = 0;

		for (i32 j = 0; j < ungrouped.size(); j++)
		{
			KBvhNode* test = ungrouped[j].get();
			if (node == test) continue;

			GBoundingBox testbox;
			testbox.Update(node->Bounds);
			testbox.Update(test->Bounds);
			if (!bestbox.IsSet() || testbox.GetArea() < bestbox.GetArea())
			{
				best = test;
				bestbox = testbox;
				bestindex = j;
			}
		}

		if (best)
		{
			UPtr<KBvhNode> newnode = std::make_unique<KBvhNode>();
			newnode->Bounds = bestbox;
			newnode->ChildA = std::move(std::move(ungrouped[i]));
			newnode->ChildB = std::move(std::move(ungrouped[bestindex]));

			VectorRemoveAt(ungrouped, i);
			if (bestindex > i) bestindex--;
			VectorRemoveAt(ungrouped, bestindex);
			i = -1;

			ungrouped.push_back(std::move(newnode));
		}
		else
		{
			// this is the head node
			Head = std::move(ungrouped[i]);
			//grouped.push_back(std::move(node));
			VectorRemoveAt(ungrouped, i);
			break;
		}
	}
}
