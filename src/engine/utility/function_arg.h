#pragma once

#include "kfglobal.h"

// replaces a lambda + std::function that will be passed as a function argument
// lambdas with captures going to std::functions require dynamic allocations
//	which are slow and boring, and can be prevented with a template
template <typename FunctionArgs, typename LocalArgs = void, typename ReturnType = void>
class KFunctionArg
{
	LocalArgs* Local = nullptr;
	ReturnType (*Func)(FunctionArgs&, LocalArgs&);
public:
	KFunctionArg() = default;
	KFunctionArg(ReturnType(*func)(FunctionArgs&, LocalArgs&), LocalArgs& localArgs) :
		Func(func), Local(&localArgs) {}

	ReturnType Execute(FunctionArgs& funcArgs)
	{
		return Func(funcArgs, *Local);
	}
};