#pragma once
#include "kfglobal.h"
#include "engine/math/aabb.h"
#include "engine/math/plane.h"

class KBspNode
{
	friend class KBspTree;
	friend class KMapCompiler;

	bool bIsLeaf = false;
	KBspNode* Parent = nullptr;
	DBoundingBox Bounds;


	// split node properties
	DPlane SplitPlane;
	UPtr<KBspNode> Front, Back;

	// leaf node properties
	TVector<UPtr<class KBrushFace>> LeafFaces; // faces form a convex shape
	TVector<class KLeafPortal*> LeafPortals; // portals are owned by the tree since leaves share them
	bool bFilled = false;

public:

	~KBspNode();

	bool IsSolidLeaf() { return bIsLeaf && LeafFaces.size() == 0; }
	bool IsFilledLeaf() { return bIsLeaf && bFilled; }
	bool IsLeaf() { return bIsLeaf; }

	const DPlane& GetPlane() { return SplitPlane; }
	KBspNode* GetFront() { return Front.get(); }
	KBspNode* GetBack() { return Back.get(); }

	const DBoundingBox& GetBounds() { return Bounds; } 
	KBspNode* GetParent() { return Parent; }

	TVector<UPtr<class KBrushFace>>& GetLeafFaces() { return LeafFaces; }
	TVector<class KLeafPortal*> GetLeafPortals() { return LeafPortals; }
};