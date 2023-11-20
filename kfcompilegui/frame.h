#pragma once

#include "wx/wx.h"

class KFrame : public wxFrame
{
public:

	class wxButton* CompileButton;
	class wxCheckBox* NoLight;
	class wxTextCtrl* CompilerOutput;
	//class wxListBox* List1;
	//class wxListBox* CompilerOutput;

	std::unique_ptr<class KCompilerProcess> CompilerProcess;

public:
	KFrame();
	~KFrame();

	void OnCompileClicked(wxCommandEvent& evt);
	void OnIdle(wxIdleEvent& evt);
	void OnCompilerTerminated(class KCompilerProcess* process);

	void AddMessage(const wxString& msg);

	wxDECLARE_EVENT_TABLE();
};
