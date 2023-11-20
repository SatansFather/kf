#pragma once

#if !_SERVER

#include "hud_widget.h"

class KHudRegion : public KHudWidget
{
	bool bScaleHorizontalBoundsWithChildren = true;
	bool bScaleVerticalBoundsWithChildren = true;
	EAlignmentType ChildPlacementOrientation;
	TVector<UPtr<KHudWidget>> Children;

public:
	
	void Draw() override;

};

#endif