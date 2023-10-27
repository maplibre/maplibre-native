#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <unordered_map>

namespace mbgl {
namespace util {

/// A wrapper around `unordered_map` which uses linear search below some threshold of size
template <typename Key,
          typename T,
          std::size_t LinearThreshold,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
struct TinyUnorderedMap : private std::unordered_map<Key, T, Hash, KeyEqual, Allocator> {
    using Super = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

public:
    using value_type = typename Super::value_type;

    TinyUnorderedMap() = default;

    TinyUnorderedMap(TinyUnorderedMap&& rhs)
        : Super(std::move(rhs)),
          linearSize(rhs.linearSize),
          keys(std::move(rhs.keys)),
          values(std::move(rhs.values)) {
        rhs.linearSize = 0;
    }
    TinyUnorderedMap(const TinyUnorderedMap& rhs)
        : Super(rhs),
          linearSize(rhs.linearSize),
          keys(rhs.keys),
          values(rhs.values) {}

    /// Construct from a range of key-value pairs
    template <typename InputIterator>
    TinyUnorderedMap(InputIterator first, InputIterator last) {
        const auto n = std::distance(first, last);
        if (n <= static_cast<decltype(n)>(LinearThreshold)) {
            for (std::size_t i = 0; first != last; i++) {
                keys[i].emplace(first->first);
                values[i].emplace(first->second);
                ++first;
            }
            linearSize = n;
        } else {
            Super{first, last}.swap(*this);
        }
    }

    /// Construct from a range of keys and a range of values.
    template <typename KeyInputIterator, typename ValueInputIterator>
    TinyUnorderedMap(KeyInputIterator firstKey,
                     KeyInputIterator lastKey,
                     ValueInputIterator firstValue,
                     [[maybe_unused]] ValueInputIterator lastValue) {
        const auto n = std::distance(firstKey, lastKey);
        assert(n == std::distance(firstValue, lastValue));
        if (n <= static_cast<decltype(n)>(LinearThreshold)) {
            for (std::size_t i = 0; firstKey != lastKey; i++) {
                keys[i].emplace(*firstKey++);
                values[i].emplace(*firstValue++);
            }
            linearSize = n;
        } else {
            this->reserve(n);
            while (firstKey != lastKey) {
                this->Super::operator[](*firstKey++) = *firstValue++;
            }
        }
    }

    /// Construct from  initializer lists
    template <typename KeyInput, typename ValueInput>
    TinyUnorderedMap(std::initializer_list<KeyInput> keys, std::initializer_list<ValueInput> values)
        : TinyUnorderedMap(keys.begin(), keys.end(), values.begin(), values.end()) {}

    TinyUnorderedMap(std::initializer_list<value_type> list)
        : TinyUnorderedMap(list.begin(), list.end()) {}

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
    TinyUnorderedMap& swap(TinyUnorderedMap& other) {
        this->Super::swap(other);
        std::swap(keys, other.keys);
        std::swap(values, other.values);
        std::swap(linearSize, other.linearSize);
        return *this;
    }

    std::size_t getThreshold() const { return LinearThreshold; }

    std::size_t size() const { return linearSize ? linearSize : Super::size(); }
    bool empty() const { return !size(); }

    using Super::begin;
    using Super::end;

    std::optional<std::reference_wrapper<T>> find(const Key& key) {
        return TinyUnorderedMap::find<TinyUnorderedMap, T>(*this, key);
    }
    std::optional<std::reference_wrapper<const T>> find(const Key& key) const {
        return TinyUnorderedMap::find<const TinyUnorderedMap, const T>(*this, key);
    }

    std::optional<std::reference_wrapper<T>> operator[](const Key& key) {
        return TinyUnorderedMap::find<TinyUnorderedMap, T>(*this, key);
    }
    std::optional<std::reference_wrapper<const T>> operator[](const Key& key) const {
        return TinyUnorderedMap::find<const TinyUnorderedMap, const T>(*this, key);
    }

    std::size_t count(const Key& key) const { return find(key) ? 1 : 0; }

private:
    std::size_t linearSize = 0;
    std::array<std::optional<Key>, LinearThreshold> keys;
    std::array<std::optional<T>, LinearThreshold> values;

    // templated to generalize `iterator` and `const_iterator` return values
    template <typename TThis, typename TRet>
    static std::optional<std::reference_wrapper<TRet>> find(TThis& map, const Key& key) {
        if (map.linearSize) {
            const auto eq = map.key_eq();
            const auto end = map.keys.begin() + map.linearSize;
            auto pred = [&](const auto& x) {
                return eq(key, *x);
            };
            if (const auto hit = std::find_if(map.keys.begin(), end, std::move(pred)); hit != end) {
                return **std::next(map.values.begin(), std::distance(map.keys.begin(), hit));
            }
        } else if (const auto hit = map.Super::find(key); hit != map.end()) {
            return hit->second;
        }
        return std::nullopt;
    }
};

} // namespace util
} // namespace mbgl
