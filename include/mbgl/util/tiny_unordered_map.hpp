#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <unordered_map>

namespace mbgl {
namespace util {

/// A wrapper around `unordered_map` which uses linear search below some threshold of size.
///
/// Currently an immutable map, not allowing insert or delete, but could be extended to allow that, with extra cost when
/// transitioning over the threshold value, of course. The maped values can be modified via reference accessors, just
/// not the set of keys or the keys themselves. Since the small-case elements are stored separately from
/// `unordered_map`, we can't use that class' iterators, and its interface is private.  A wrapper iterator might allow
/// for a better and more complete public interface.
/// @tparam Key The key type
/// @tparam T The mapped type
/// @tparam LinearThreshold The count of elements above which a dynamically allocated hashtable is used
/// @tparam Hash The hash implementation
/// @tparam KeyEqual The key equality implementation
/// @tparam Allocator The allocator used for dynamic key-value-pair allocations only
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

    /// Construct from a range of key-value pairs
    template <typename InputIterator>
    TinyUnorderedMap(InputIterator first, InputIterator last)
        : TinyUnorderedMap(first, last, [](auto& x) { return x; }) {}

    /// Construct from a generator
    template <typename InputIterator, typename GeneratorFunction>
    TinyUnorderedMap(InputIterator first, InputIterator last, GeneratorFunction gen) {
        assign(first, last, gen);
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
    TinyUnorderedMap(std::initializer_list<KeyInput> keys_, std::initializer_list<ValueInput> values_)
        : TinyUnorderedMap(keys_.begin(), keys_.end(), values_.begin(), values_.end()) {}

    TinyUnorderedMap(std::initializer_list<value_type> values_)
        : TinyUnorderedMap(values_.begin(), values_.end()) {}

    /// Move constructor
    TinyUnorderedMap(TinyUnorderedMap&& rhs)
        : linearSize(rhs.linearSize),
          keys(std::move(rhs.keys)),
          values(std::move(rhs.values)) {
        rhs.linearSize = 0;
        Super::operator=(std::move(rhs));
    }

    /// Copy constructor
    TinyUnorderedMap(const TinyUnorderedMap& rhs)
        : Super(rhs),
          linearSize(rhs.linearSize),
          keys(rhs.keys),
          values(rhs.values) {}

    /// Replace contents from generator
    template <typename InputIterator, typename GeneratorFunction>
    TinyUnorderedMap& assign(InputIterator first, InputIterator last, GeneratorFunction gen) {
        const auto n = std::distance(first, last);
        if (n <= static_cast<decltype(n)>(LinearThreshold)) {
            this->Super::clear();
            for (std::size_t i = 0; first != last; i++) {
                auto result = gen(*first++);
                keys[i].emplace(std::move(result.first));
                values[i].emplace(std::move(result.second));
            }
            linearSize = n;
        } else {
            linearSize = 0;
            this->reserve(n);
            while (first != last) {
                this->Super::insert(gen(*first++));
            }
        }
        return *this;
    }

    /// Copy assignment
    TinyUnorderedMap& operator=(const TinyUnorderedMap& rhs) {
        TinyUnorderedMap{rhs}.swap(*this);
        return *this;
    }

    /// Move assignment
    TinyUnorderedMap& operator=(TinyUnorderedMap&& rhs) noexcept {
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

    std::size_t size() const noexcept { return linearSize ? linearSize : Super::size(); }
    bool empty() const noexcept { return !size(); }

    std::optional<std::reference_wrapper<T>> find(const Key& key) noexcept {
        return TinyUnorderedMap::find<TinyUnorderedMap, T>(*this, key);
    }
    std::optional<std::reference_wrapper<const T>> find(const Key& key) const noexcept {
        return TinyUnorderedMap::find<const TinyUnorderedMap, const T>(*this, key);
    }

    std::optional<std::reference_wrapper<T>> operator[](const Key& key) noexcept {
        return TinyUnorderedMap::find<TinyUnorderedMap, T>(*this, key);
    }
    std::optional<std::reference_wrapper<const T>> operator[](const Key& key) const noexcept {
        return TinyUnorderedMap::find<const TinyUnorderedMap, const T>(*this, key);
    }

    std::size_t count(const Key& key) const noexcept { return find(key) ? 1 : 0; }

    void clear() {
        linearSize = 0;
        this->Super::clear();
    }

    bool operator==(const TinyUnorderedMap& other) const {
        if (size() != other.size()) return false;
        assert(linearSize == other.linearSize);
        if (linearSize) {
            return std::equal(
                       keys.begin(), keys.begin() + linearSize, other.keys.begin(), other.keys.begin() + linearSize) &&
                   std::equal(values.begin(),
                              values.begin() + linearSize,
                              other.values.begin(),
                              other.values.begin() + linearSize);
        }
        return std::operator==(*this, other);
    }

private:
    std::size_t linearSize = 0;
    std::array<std::optional<Key>, LinearThreshold> keys;
    std::array<std::optional<T>, LinearThreshold> values;

    auto makeFindPredicate(const Key& key) const noexcept {
        return [&, eq = this->Super::key_eq()](const auto& x) {
            return eq(key, *x);
        };
    }

    /// Search the small-map storage for a key.
    /// @return A pair of `bool found` and `size_t index`
    auto findLinear(const Key& key) const noexcept {
        const auto beg = keys.begin();
        const auto end = beg + linearSize;
        const auto hit = std::find_if(beg, end, makeFindPredicate(key));
        return std::make_pair(hit != end, hit != end ? std::distance(beg, hit) : 0);
    }

    // templated to provide `iterator` and `const_iterator` return values without duplication
    template <typename TThis, typename TRet>
    static std::optional<std::reference_wrapper<TRet>> find(TThis& map, const Key& key) noexcept {
        if (map.linearSize) {
            if (const auto result = map.findLinear(key); result.first) {
                // Return a reference to the value at the same index
                assert(map.values[result.second]);
                return *map.values[result.second];
            }
        } else if (const auto hit = map.Super::find(key); hit != map.end()) {
            return hit->second;
        }
        return std::nullopt;
    }
};

} // namespace util
} // namespace mbgl
