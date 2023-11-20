#pragma once

#if !_SERVER && _WIN32

// gdi header doesnt like to come after win32 lean and mean
void CallInitGDI();

#endif