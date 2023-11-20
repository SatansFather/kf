#if !_SERVER && _WIN32

#include "gdi_init.h"
#include "engine/os/windows/gdi.h"

void CallInitGDI()
{
	InitGDI();
}

#endif

