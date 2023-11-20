#pragma once

#include <ostream>
#include "engine/os/windows//windows.h"

inline std::ostream& console_blue(std::ostream& s)
{
#if _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
		| FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
	return s;
}

inline std::ostream& console_red(std::ostream& s)
{
#if _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
	return s;
}

inline std::ostream& console_green(std::ostream& s)
{
#if _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
	return s;
}

inline std::ostream& console_yellow(std::ostream& s)
{
#if _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
	return s;
}

inline std::ostream& console_white(std::ostream& s)
{
#if _WIN32
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	return s;
}

#if _WIN32

struct FConsoleColor
{
	FConsoleColor(WORD attribute) : Color(attribute) {};
	WORD Color;
};

template <class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>&
operator<<(std::basic_ostream<_Elem, _Traits>& i, FConsoleColor& c)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, c.Color);
	return i;
}

#endif
