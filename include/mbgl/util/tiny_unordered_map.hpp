#pragma once

#include <algorithm>
#include <unordered_map>

namespace mbgl {
namespace util {

/// A wrapper around `unordered_map` which uses linear search below some threshold of size
template <typename Key,
          typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
struct TinyUnorderedMap : public std::unordered_map<Key, T, Hash, KeyEqual, Allocator> {
    using Super = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

public:
    using value_type = Super::value_type;

    TinyUnorderedMap(std::size_t linearThreshold_ = 0)
        : linearThreshold(linearThreshold_) {}
    TinyUnorderedMap(TinyUnorderedMap&& rhs)
        : Super(std::move(rhs)),
          linearThreshold(rhs.linearThreshold) {}
    TinyUnorderedMap(const TinyUnorderedMap& rhs)
        : Super(rhs),
          linearThreshold(rhs.linearThreshold) {}

    /// Construct from a range of keys and a range of values.
    template <typename KeyInputIterator, typename ValueInputIterator>
    TinyUnorderedMap(std::size_t linearThreshold_,
                     KeyInputIterator firstKey,
                     KeyInputIterator lastKey,
                     ValueInputIterator firstValue,
                     [[maybe_unused]] ValueInputIterator lastValue)
        : linearThreshold(linearThreshold_) {
        assert(std::distance(firstKey, lastKey) == std::distance(firstValue, lastValue));
        this->reserve(std::distance(firstKey, lastKey));
        while (firstKey != lastKey) {
            this->operator[](*firstKey++) = *firstValue++;
        }
    }

    /// Construct from  initializer lists
    template <typename KeyInput, typename ValueInput>
    TinyUnorderedMap(std::size_t linearThreshold_,
                     std::initializer_list<KeyInput> keys,
                     std::initializer_list<ValueInput> values)
        : TinyUnorderedMap(linearThreshold_, keys.begin(), keys.end(), values.begin(), values.end()) {}

    TinyUnorderedMap(std::size_t linearThreshold_, std::initializer_list<value_type> list)
        : Super(list.begin(), list.end()),
          linearThreshold(linearThreshold_) {}

    std::size_t getThreshold() const { return linearThreshold; }
    void setThreshold(std::size_t value) { linearThreshold = value; }

    auto find(const Key& key) { return TinyUnorderedMap::find(*this, key); }
    auto find(const Key& key) const { return TinyUnorderedMap::find(*this, key); }

    /// copy assignment
    TinyUnorderedMap& operator=(const TinyUnorderedMap& rhs) {
        TinyUnorderedMap{rhs}.swap(*this);
        return *this;
    }
    /// move assignment
    TinyUnorderedMap& operator=(TinyUnorderedMap&& rhs) {
        TinyUnorderedMap{std::move(rhs)}.swap(*this);
        return *this;
    }

    /// Swap all contents with another instance
    void swap(TinyUnorderedMap& other) {
        std::swap(static_cast<Super&>(*this), static_cast<Super&>(other));
        std::swap(linearThreshold, other.linearThreshold);
        return *this;
    }

private:
    std::size_t linearThreshold = 0;

    // templated to generalize `iterator` and `const_iterator` return values
    template <typename TThis>
    static auto find(TThis& map, const Key& key) {
        if (map.size() <= map.linearThreshold) {
            auto eq = map.key_eq();
            return std::find_if(map.begin(), map.end(), [&](const auto& x) { return eq(key, x.first); });
        } else {
            return map.Super::find(key);
        }
    }
};

} // namespace util
} // namespace mbgl
