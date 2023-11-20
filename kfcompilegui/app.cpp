#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(KApp);

KApp::KApp() {}
KApp::~KApp() {}

bool KApp::OnInit()
{
	MainFrame = new KFrame();
	MainFrame->Show();
	return true;
}
