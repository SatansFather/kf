#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <deque>
#include <forward_list>
#include <list>
#include <ostream>

template <typename T>
using TVector = std::vector<T>;

template <typename A, typename B>
using THashMap = std::unordered_map<A, B>;

template <typename A, typename B>
using TMap = std::map<A, B>;

template <typename T>
using TSet = std::set<T>;

template <typename T>
using THashSet = std::unordered_set<T>;

template <typename T>
using TList = std::list<T>;

template <typename T>
using TForwardList = std::forward_list<T>;

template <typename T>
using TDeque = std::deque<T>;

template <typename... TArgs>
using TTuple = std::tuple<TArgs...>;

template <typename T>
static std::ostream& operator << (std::ostream& os, const TVector<T>& v)
{
	for (const T& t : v)
	{
		os << t << "\n";
	}
	return os;
}

#define VectorContains(vec, el) (std::find(vec.begin(), vec.end(), el) != vec.end())
#define VectorRemove(arr, val) (arr.erase(std::remove(arr.begin(), arr.end(), val), arr.end()))
#define VectorRemoveAt(arr, idx) (arr.erase(arr.begin() + idx))
#define VectorAddUnique(arr, val) { if (std::find(arr.begin(), arr.end(), val) == arr.end()) arr.push_back(val); }

#define MapRemoveTo_Exclusive(m, v) m.erase(m.begin(), m.lower_bound(v));
#define MapRemoveTo_Inclusive(m, v) m.erase(m.begin(), m.upper_bound(v));

// variadic args for lambda capture
#define TUPLE_ITER_START(varName, capture) \
[&] <std::size_t... Is> (std::index_sequence<Is...>) { \
auto ___TUPLE_ITER_ = capture <std::size_t I> (auto && varName) {

#define TUPLE_ITER_END(tupleName, tupleType) \
};(___TUPLE_ITER_.template operator() < Is > (std::get<Is>(tupleName)), ...); \
}(std::make_index_sequence<std::tuple_size_v<std::decay_t<std::tuple<tupleType...>>>>{});

#define TUPLE_FOR_EACH(tupleName, tupleType, iterVar, capture, ...) TUPLE_ITER_START(iterVar, capture) __VA_ARGS__ TUPLE_ITER_END(tupleName, tupleType)