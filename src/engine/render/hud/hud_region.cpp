#if !_SERVER && !_COMPILER
#include "hud_region.h"

void KHudRegion::Draw()
{
	for (UPtr<KHudWidget>& w : Children)
		w->Draw();
}

#endif
