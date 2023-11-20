#pragma once

#if _DEV || _DEBUG
#define K_ASSERT(condition, msg) {if (!(condition)) KAssertion(msg);}
#define K_ASSERT_REL(condition, msg) K_ASSERT(condition, msg)
#else
#define K_ASSERT(condition, msg){if (!(condition)) KAssertion(msg);}
#define K_ASSERT_REL(condition, msg) // this one does whatever K_ASSERT does in dev
#endif

// check an HRESULT
#if _WIN32
#include "engine/os/windows/windows.h"
#define K_ASSERT_HR(hr, msg) { HRESULT __hr__ = hr; if (FAILED(__hr__)) KAssertion(__hr__, msg); }
#endif

struct KAssertion
{
#if _WIN32
	KAssertion(HRESULT hresult, const char* msg);
#endif

	KAssertion(const char* msg);
};