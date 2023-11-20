#include "listener.h"
#include "input_processor.h"

void KInputListener::GetCursorPosition(f32& x, f32& y) const
{
#if !_SERVER
	KInputProcessor::GetCursorPos(x, y);
#endif
}
