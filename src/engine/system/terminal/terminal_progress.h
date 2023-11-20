#pragma once

#include "engine/global/types_numeric.h"
#include "engine/utility/kstring.h"
#include "engine/system/time.h"

class KTerminalProgressBar
{
private:

	u8 Percent;
	bool bOneLine = false;
	bool bFinished = false;
	KTimePoint StartTime;
	KString OperationText;

public:
	
	KTerminalProgressBar() = default;
	KTerminalProgressBar(KString text, bool oneline = true, f32 r = 1, f32 g = 1, f32 b = 1);
	void UpdateProgress(f32 current, f32 total);
	void Finish();

private:

	void UpdateTerminal(f32 progress);
};