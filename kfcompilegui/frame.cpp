#include "frame.h"
#include "process.h"

#define COMPILE_BUTTON_ID 1

wxBEGIN_EVENT_TABLE(KFrame, wxFrame)
	EVT_BUTTON(COMPILE_BUTTON_ID, OnCompileClicked)
	//EVT_IDLE(KFrame::OnIdle)
wxEND_EVENT_TABLE()

KFrame::KFrame() : wxFrame(nullptr, wxID_ANY, "Karnage Freak Compiler GUI", 
	wxPoint(30, 30), wxSize(800, 600), wxCLOSE_BOX | wxMINIMIZE | wxCAPTION | wxCLIP_CHILDREN)
{
	CompileButton = new wxButton(this, COMPILE_BUTTON_ID, "compile", wxPoint(10, 10), wxSize(150, 50));
	CompilerOutput = new wxTextCtrl(this, wxID_ANY, "", wxPoint(10, 70), wxSize(700, 400), wxTE_RICH | wxTE_MULTILINE);
	NoLight = new wxCheckBox(this, wxID_ANY, "Full Light", wxPoint(180, 10));
	//CompilerOutput = new wxListBox(this, wxID_ANY, wxPoint(10, 110), wxSize(700, 500));
	wxFont consolas(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	CompilerOutput->SetFont(consolas);
}

KFrame::~KFrame()
{

}

void KFrame::OnCompileClicked(wxCommandEvent& evt)
{
#if 1
	//List1->AppendString(Text1->GetValue());
	wxString mapname = CompilerOutput->GetLineText(0);
	wxString cmd = "D:/karnagefreak/out/build/x64-release/kfcompiledev.exe -map=D:/karnagefreak/res/maps/" + mapname + ".map -newconsole";
	CompilerProcess = std::make_unique<KCompilerProcess>(this, cmd);
	if (NoLight->IsChecked())
		cmd += " -nolight";

	wxString wd = wxGetCwd();
	wxSetWorkingDirectory("D:/karnagefreak/out/build/x64-release/");
	wxExecute(cmd, wxEXEC_ASYNC);//, CompilerProcess.get());
	wxSetWorkingDirectory(wd);
	evt.Skip();
#else
	//List1->AppendString(Text1->GetValue());
	wxString mapname = CompilerOutput->GetLineText(0); 
	//wxString cmd = "D:/karnagefreak/out/build/x64-release/kfcompiledev.exe -map=D:/karnagefreak/res/maps/" + mapname + ".map -newconsole";
	wxString cmd = "kfcompile.exe -map=maps/" + mapname + ".map -newconsole";
	if (NoLight->IsChecked())
		cmd += " -nolight";
	
	CompilerProcess = std::make_unique<KCompilerProcess>(this, cmd);

	//wxString wd = wxGetCwd();
	//wxSetWorkingDirectory("D:/karnagefreak/out/build/x64-release/");
	wxExecute(cmd, wxEXEC_ASYNC);//, CompilerProcess.get());
	//wxSetWorkingDirectory(wd);
	evt.Skip();
#endif
	//List1->AppendString("fukky");
}

void KFrame::OnIdle(wxIdleEvent& evt)
{
	if (CompilerProcess && CompilerProcess->HasInput())
		evt.RequestMore();
}

void KFrame::OnCompilerTerminated(class KCompilerProcess* process)
{
	
}

void KFrame::AddMessage(const wxString& msg)
{
	int lineCount = CompilerOutput->GetNumberOfLines();
	wxString lastLine = CompilerOutput->GetLineText(lineCount - 2);
	wxString lastMsg = lastLine.BeforeFirst('[');

	//CompilerOutput->AppendText(lastMsg + "-------------" + msg.BeforeFirst('[') + "\n");

	if (lastMsg == msg.BeforeFirst('['))
	{
		int startLastLine = CompilerOutput->XYToPosition(0, lineCount - 2);
		int endLastLine = CompilerOutput->GetLastPosition() + 1;
		CompilerOutput->Remove(startLastLine, endLastLine);
		//CompilerOutput->AppendText("MYCORN\n");
		//return;
	}

	CompilerOutput->AppendText(msg + "\n");
/*
	wxString lastMsg = CompilerOutput->GetString(CompilerOutput->GetCount() - 1);
	lastMsg = lastMsg.BeforeFirst('[');
	if (lastMsg == msg.BeforeFirst('['))
	{
		CompilerOutput->SetString(CompilerOutput->GetCount() - 1, msg);
		return;
	}
	
	CompilerOutput->AppendString(msg);*/
}
