#pragma once

#include "kfglobal.h"

struct KPendingConsoleCommand
{
	std::function<KString(const KString&)> Function;
	KString Value;
};