#pragma once

#if _WIN32 && !_SERVER && !_COMPILER

// splash screen on startup before main window shows on screen
class KSplashScreen
{
public:
	KSplashScreen();
	~KSplashScreen();

#if _PACK
	static void WriteImageToHeader();
#endif
};

#endif