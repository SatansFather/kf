#pragma once

#include "engine/utility/kstring.h"
#include "terminal_color.h"
#include <iostream>

#define SYSLOG_COLOR(t, color) { std::cout << color << t << "\n"; } 

#define SYSLOG(t) SYSLOG_COLOR(t, console_white)
#define SYSLOG_WARNING(t) SYSLOG_COLOR(t, console_yellow)
#define SYSLOG_ERROR(t) SYSLOG_COLOR(t, console_red)
#define SYSLOG_WHITE(t) SYSLOG_COLOR(t, console_white);
#define SYSLOG_GREEN(t) SYSLOG_COLOR(t, console_green);

/*
#if _SERVER
#define LOG(t)			SYSLOG(t)
#define LOG_WARNING(t)	SYSLOG_WARNING(t)
#define LOG_ERROR(t)	SYSLOG_ERROR(t)
#else
#define LOG(t)			(Console->ConsolePrint(t, 1, 1, 1))
#define LOG_WARNING(t)	(Console->ConsolePrint(t, 1, 1, 0))
#define LOG_ERROR(t)	(Console->ConsolePrint(t, 1, 0, 0))
#endif*/

#if _DEBUG
#if _SERVER
#define LOG_DEBUG(t)			SYSLOG(t)		
#define LOG_WARNING_DEBUG(t)	SYSLOG_WARNING(t)
#define LOG_ERROR_DEBUG(t)		SYSLOG_ERROR(t)
#else
#define LOG_DEBUG(t)			LOG(t)	
#define LOG_WARNING_DEBUG(t)	LOG_WARNING(t)
#define LOG_ERROR_DEBUG(t)		LOG_ERROR(t)
#endif
#else
#define LOG_DEBUG(t)
#define LOG_WARNING_DEBUG(t)
#define LOG_ERROR_DEBUG(t)
#endif

class KSystemTerminal
{
private:
	
	static bool bIsShowing;

public:

	static void SetShowing(bool showing);
	static bool IsShowing();

	static void Show();
	static void Hide();
};
