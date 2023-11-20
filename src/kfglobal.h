#pragma once

#include "engine/global/types.h"
#include "engine/utility/kstring.h"
#include "engine/system/time.h"
#include "engine/kpool/include/kpool.h"
#include <algorithm>
#include <array>
#include <functional>
#include <iosfwd>
#include <cstring>



template <typename T>
using TObjRef = KPool::ObjRef<T>;

template <typename T>
using TDataPool = KPool::DataPool<T>;

typedef KPool::Poolable KPoolable;
typedef KPool::DataPoolBase KDataPoolBase;

inline TVector<KDataPoolBase*> RenderableDataPools;
inline TVector<KDataPoolBase*> EntityDataPools;
inline TVector<KDataPoolBase*> SnapshotDataPools;
inline TVector<KDataPoolBase*> HistoryDataPools;

#define NULL_PLAYER 255

extern GFlt GameFrameDelta();
extern GFlt GetFrameAlpha();
extern GFlt MatchLifeTime();

extern bool IsNetAuthority();
extern bool IsNetClient();
extern bool IsNetServer();

extern bool IsReadingReplay();
extern bool IsWritingReplay();

extern void _SYSLOG(const KString& message);
extern void _SYSLOG_WARNING(const KString& message);
extern void _SYSLOG_ERROR(const KString& message);

#if !_SERVER && !_COMPILER && !_PACK
extern void LOG(const KString& message, GFlt r = 1, GFlt g = 1, GFlt b = 1);
extern void CHATLOG(const KString& message, GFlt r = 1, GFlt g = 1, GFlt b = 1);
#else
static void LOG(const KString& message, GFlt r = 1, GFlt g = 1, GFlt b = 1)
{
	_SYSLOG(message);
}
static void CHATLOG(const KString& message, GFlt r = 1, GFlt g = 1, GFlt b = 1)
{
	_SYSLOG(message);
}
#endif

// allows string literals as a template argument
// template <FixedString str> to declare
// ClassName<"xxxx"> to instantiate
template<unsigned N>
struct StringLiteral
{
	char buf[N + 1]{};
	constexpr StringLiteral(char const* s)
	{
		for (unsigned i = 0; i != N; ++i) buf[i] = s[i];
	}
	constexpr operator char const* () const { return buf; }

	auto operator<=>(const StringLiteral&) const = default;
};
template<unsigned N> StringLiteral(char const (&)[N])->StringLiteral<N - 1>;

#define IS_OVERRIDDEN(base, child, func)                                 \
(std::is_base_of<base, child>::value                                     \
 && !std::is_same<decltype(&base::func), decltype(&child::func)>::value)

#define TYPE_HAS_FUNC(type, func) ([]<typename T>()->bool{constexpr bool h = requires(const T & t) {t.func();};return h;}.template operator()<type>())
#define CALL_FUNC_IF_HAS(type, func, var) ([]<typename T>(T* v)->bool{constexpr bool h = requires(const T & t) {t.func();};if constexpr (h) v->func(); return h;}.template operator()<type>(var))
#define TYPE_HAS_FUNC_OUT(type, func, outType) ([]<typename T>()->bool{constexpr bool h = requires(const T& t, outType& o) {t.func(o);};return h;}.template operator()<type>())
#define CALL_FUNC_IF_HAS_OUT(type, func, var, outType, out) {if constexpr (TYPE_HAS_FUNC_OUT(type, func, outType)) {([]<typename T>(T* v, outType& out)->void{constexpr bool h = requires(const T& t, outType& o) {t.func(o);};if(h){v->func(out);}}.template operator()<type>(var, out));}}