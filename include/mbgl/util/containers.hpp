#pragma once
#if MLN_USE_UNORDERED_DENSE
#include <ankerl/unordered_dense.h>
#else
#include <unordered_map>
#include <unordered_set>
#endif

namespace mbgl {
#if MLN_USE_UNORDERED_DENSE
template <typename Key, typename Value, typename Hash = std::hash<Key>>
using unordered_map = ankerl::unordered_dense::map<Key, Value, Hash>;

template <typename Value>
using unordered_set = ankerl::unordered_dense::set<Value>;
#else
template <typename Key, typename Value, typename Hash = std::hash<Key>>
using unordered_map = std::unordered_map<Key, Value, Hash>;

template <typename Value>
using unordered_set = std::unordered_set<Value>;
#endif
} // namespace mbgl
