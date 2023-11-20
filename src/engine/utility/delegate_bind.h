// TODO delete this file

#pragma once

#include "engine/utility/macro.h"

// bind macros use "this" and should be called from a member function of the object who owns the listener
#define BIND_DELEGATE(func, listener, broadcaster)	{ listener.Bind(std::bind(&func, this), broadcaster); }

// va params are the argument TYPES
#define BIND_DELEGATE_PARAMS( func, listener, broadcaster ... ) VA_SELECT( BIND_DELEGATE_PARAMS, __VA_ARGS__ )

#define BIND_DELEGATE_PARAMS_1( a )					\
{ listener.Bind(std::bind(&func, this, ), broadcaster, std::placeholders::_1); }

#define BIND_DELEGATE_PARAMS_2( a, b )
#define BIND_DELEGATE_PARAMS_3( a, b, c )