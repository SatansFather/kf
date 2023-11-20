#pragma once

#if !_SERVER && !_COMPILER

#include "../../hud_widget.h"
#include "../../alignment.h"

class KHudControl_Clickable : public KHudWidget
{
	friend class KRenderInterface;

public:
	
	std::function<void()> OnClicked;

};

#endif