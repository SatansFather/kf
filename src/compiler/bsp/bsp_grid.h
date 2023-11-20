#pragma once

#include "kfglobal.h"
#include "bsp_tree.h"

class KBspGrid
{
	const u32 Dimension = 256;
	TVector<TVector<TVector<KBspTree>>> TreeGrid;

public:

	KBspGrid() = default;
	KBspGrid(TVector<UPtr<class KBrushFace>>& faces, u32 dimension = 256);

};