#include "process.h"
#include "wx/txtstrm.h"
#include "frame.h"

void KCompilerProcess::OnTerminate(int pid, int status)
{
	Frame->OnCompilerTerminated(this);
	delete this;
}

bool KCompilerProcess::HasInput()
{
	if (IsInputAvailable())
	{
		if (!GetInputStream()) return false;
		wxTextInputStream st(*GetInputStream());

		wxString msg;
		msg << st.ReadLine();
		Frame->AddMessage(msg);
		return true;
	}
	return false;
}
