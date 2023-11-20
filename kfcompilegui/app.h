#pragma once

#include "wx/wx.h"
#include <memory>

class KApp : public wxApp
{
	class KFrame* MainFrame = nullptr;

public:
	KApp();
	~KApp();
	
	bool OnInit() override;
};
