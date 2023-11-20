#pragma once

#if !_SERVER && !_COMPILER

#include "hud_widget.h"

class KHudOnScreenMessages : public KHudWidget
{
	friend class KRenderInterface;

	TVector<struct KOnScreenMessage> Messages;

public:

	KHudOnScreenMessages();
	void Draw() override;
	void SubmitPending(TVector<struct KOnScreenMessage>& pending);
};

#endif