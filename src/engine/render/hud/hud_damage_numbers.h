#pragma once

#if !_SERVER

#include "hud_widget.h"

class KHudDamageNumbers : public KHudWidget
{
	
	TMap<u32, u32> DamageToLayoutMap;

public:

	KHudDamageNumbers();
	void Draw() override;
	void OnWindowResize() override;
};


#endif