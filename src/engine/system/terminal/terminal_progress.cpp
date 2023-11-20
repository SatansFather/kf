
#include "terminal_progress.h"
#include "terminal.h"
#include <iomanip>
#include "engine/system/time.h"
#include "engine/run/k_main.h"

#if _WIN32
#include <consoleapi2.h>
#endif

KTerminalProgressBar::KTerminalProgressBar(KString text, bool oneline, f32 r /*= 1*/, f32 g /*= 1*/, f32 b /*= 1*/)
{
	OperationText = text;
	i32 extra = 25 - OperationText.Length();
	if (extra > 0) 
		for (i32 i = 0; i < extra; i++)
			OperationText += ".";
	
	bOneLine = oneline;
	StartTime = KTime::Now();
	UpdateProgress(0, 1);
}

void KTerminalProgressBar::UpdateProgress(f32 current, f32 total)
{
	if (bFinished) return;

	u8 pre = Percent;
	Percent = (current / total) * 100;
	if (pre != Percent)
	{
		UpdateTerminal(f32(current) / f32(total));
	}
}

void KTerminalProgressBar::Finish()
{
	bFinished = true;
	Percent = 100;

	//Sys_ShowProgressBar(1, 1);
	UpdateTerminal(2);
	//SYSLOG("\n" << text << std::fixed << std::setprecision(3) << " finished in " << KTime::Since(StartTime) << " seconds.");
}

void KTerminalProgressBar::UpdateTerminal(f32 progress)
{
	bool finished = progress > 1;
	if (progress > 1) progress = 1;

	i32 bar_width = 20;

#if _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	i32 max_width = csbi.srWindow.Right - csbi.srWindow.Left - 10;
	if (bar_width > max_width) bar_width = max_width;
#endif

	if (bar_width > 70) bar_width = 70;

	KString bar(OperationText);
	bar.Reserve(OperationText.Length() + bar_width);

	bar += "[";
	i32 pos = bar_width * progress;
	for (i32 i = 0; i < bar_width; ++i)
	{
		if (i < pos) bar += "=";
		else if (i == pos) bar += ">";
		else bar += " ";
	}

	bar += "] " + KString(i32(progress * 100.0)) + "%";
	
	std::cout << console_white;

	if (!finished)
	{
		bool redirect = KApplication::HasCommandLineArg("redirect");
		std::cout << bar << (redirect ? "\n" : "\r");
	}
	else
		std::cout << bar << " - took " << std::setprecision(3) << KTime::Since(StartTime) << " seconds\n";
}
