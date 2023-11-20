#pragma once

#include "wx/process.h"

class KCompilerProcess : public wxProcess
{
	class KFrame* Frame = nullptr;
	wxString Command;

public:

	KCompilerProcess(class KFrame* frame, const wxString& cmd)
	{
		Frame = frame;
		Command = cmd;
		Redirect();
	}

	void OnTerminate(int pid, int status) override;
	bool HasInput();
};
