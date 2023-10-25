#pragma once

namespace mbgl {
namespace util {

namespace detail {
template <class Value, class C>
struct TinyMapCompare : public C {
    typedef std::pair<typename C::first_argument_type, Value> Data;
    typedef typename C::first_argument_type first_argument_type;

    TinyMapCompare() = default;
    TinyMapCompare(const C& src)
        : C(src) {}
    TinyMapCompare(C&& src)
        : C(std::move(src)) {}

    bool operator()(const Data& lhs, const first_argument_type& rhs) const { return operator()(lhs.first, rhs); }
    bool operator()(const first_argument_type& lhs, const first_argument_type& rhs) const {
        return C::operator()(lhs, rhs);
    }
};
} // namespace detail

/// A key-value map based on vectors, optimized for small collections with small keys, and optionally sorted.
/// Loosly based on Andrei Alexandrescu's `loki::AssocVector`
/// The public interface is similar to `std::map`/`std::unordered_map`.
template <typename K, typename V, bool Sorted, typename C = std::less<K>>
class TinyMap {
    using KeyVec = std::vector<K>;
    using ValueVec = std::vector<V>;
    using KeyValueVec = std::vector<std::pair<K, V>>;
    using KeyIter = typename KeyVec::iterator;
    using KeyCIter = typename KeyVec::const_iterator;
    using KeyRIter = typename KeyVec::reverse_iterator;
    using KeyCRIter = typename KeyVec::const_reverse_iterator;
    using ValueIter = typename ValueVec::iterator;
    using ValueCIter = typename ValueVec::const_iterator;
    using RefPair = std::pair<std::reference_wrapper<const K>, std::reference_wrapper<V>>;
    using CRefPair = std::pair<std::reference_wrapper<const K>, std::reference_wrapper<const V>>;

private:
    // Find the item, returning where the item should be if missing in sorted mode
    // Templated to handle `iterator` vs. `const_iterator` with less duplication.
    template <typename T>
    static auto find(T& map, const K&);

    // Find the item, returning `end()` if not found
    template <typename T>
    static auto find_eq(T& map, const K&);

public:
    using key_type = K;
    using mapped_type = V;
    using value_type = typename KeyValueVec::value_type;
    using key_compare = C;
    using size_type = typename KeyVec::size_type;

    struct iterator;
    struct const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit TinyMap(const key_compare& comp_ = key_compare())
        : comp(comp_) {}

    /// Construct from a range of key-value pairs
    template <typename InputIterator>
    TinyMap(InputIterator firstKeyValuePair, InputIterator lastKeyValuePair, const key_compare& comp_ = key_compare())
        : comp(comp_) {
        reserve(std::distance(firstKeyValuePair, lastKeyValuePair));
        while (firstKeyValuePair != lastKeyValuePair) {
            insert(*firstKeyValuePair++);
        }
    }

    /// Construct from a range of keys and a range of values.
    template <typename KeyInputIterator, typename ValueInputIterator>
    TinyMap(KeyInputIterator firstKey,
            KeyInputIterator lastKey,
            ValueInputIterator firstValue,
            [[maybe_unused]] ValueInputIterator lastValue,
            const key_compare& comp_ = key_compare())
        : comp(comp_) {
        assert(std::distance(firstKey, lastKey) == std::distance(firstValue, lastValue));
        reserve(std::distance(firstKey, lastKey));
        while (firstKey != lastKey) {
            insert(*firstKey++, *firstValue++);
        }
    }

    /// Construct from an initializer list of key-value pairs
    TinyMap(std::initializer_list<value_type> list, const key_compare& comp_ = key_compare())
        : TinyMap(list.begin(), list.end(), comp) {}

    /// Construct from an initializer list of keys and one of values
    template <typename KeyInput, typename ValueInput>
    TinyMap(std::initializer_list<KeyInput> keys,
            std::initializer_list<ValueInput> values,
            const key_compare& comp_ = key_compare())
        : TinyMap(keys.begin(), keys.end(), values.begin(), values.end(), comp) {}

    /// copy
    TinyMap(const TinyMap& rhs)
        : keys(rhs.keys),
          values(rhs.values),
          comp(rhs.comp) {}
    /// move
    TinyMap(TinyMap&& rhs)
        : keys(std::move(rhs.keys)),
          values(std::move(rhs.values)),
          comp(std::move(rhs.comp)) {}

    /// copy assignment
    TinyMap& operator=(const TinyMap& rhs) {
        TinyMap{rhs}.swap(*this);
        return *this;
    }
    /// move assignment
    TinyMap& operator=(TinyMap&& rhs) {
        TinyMap{std::move(rhs)}.swap(*this);
        return *this;
    }

    // Iterate as key-value pairs
    iterator begin() { return makeIter(keys.begin()); }
    const_iterator begin() const { return makeIter(keys.cbegin()); }
    iterator end() { return makeIter(keys.end()); }
    const_iterator end() const { return makeIter(keys.end()); }
    reverse_iterator rbegin() { return makeIter(keys.rbegin()); }
    const_reverse_iterator rbegin() const { return makeIter(keys.rbegin()); }
    reverse_iterator rend() { return makeIter(keys.rend()); }
    const_reverse_iterator rend() const { return makeIter(keys.rend()); }

    bool empty() const { return keys.empty(); }
    size_type size() const { return keys.size(); }
    size_type max_size() const { return std::min(keys.max_size(), values.max_size()); }

    bool isSorted() const { return Sorted; }

    void reserve(std::size_t sz) {
        keys.reserve(sz);
        values.reserve(sz);
    }

    /// Look up a key, inserting the default value if it's not present.
    /// Linear or log complexity on search, linear complexity on insert.
    /// Invalidates all iterators.
    mapped_type& operator[](const key_type& key) { return insert(key, mapped_type{}).first->second; }

    /// Look up and insert a copy of the specified key and value if not present.
    /// @return as `std::map::insert`
    /// Linear or log complexity on search, linear complexity on insert.
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(const value_type& kvp) { return insert(kvp.first, kvp.second); }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Linear or log complexity on search, linear complexity on insert.
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(value_type&& kvp) { return insert(std::move(kvp.first), std::move(kvp.second)); }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Linear or log complexity on search, linear complexity on insert.
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(std::pair<const key_type&, mapped_type&&> kvp) {
        return insert(kvp.first, std::move(kvp.second));
    }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(const key_type& key, mapped_type value) {
        auto i = find(*this, key);
        if (i == keys.end() || (Sorted && comp(key, *i))) {
            values.insert(keyToValueIter(i), std::move(value));
            return std::make_pair(makeIter(keys.insert(i, key)), true);
        } else {
            return std::make_pair(makeIter(i), false);
        }
    }

    /// Hinted insert
    /// Invalidates all iterators.
    iterator insert(iterator pos, const value_type& val);

    /// Insert a range of values
    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    /// Remove the key/value associated with the specified iterator.
    /// @return a replacement for the given iterator, all other iterators are invalidated
    /// Linear complexity on the number of keys following the one removed. (use range erase if possible)*
    iterator erase(iterator pos);

    /// Remove the matching key, if present.
    /// @return the number of keys removed (0 or 1), all iterators are invalidated
    /// Linear complexity on the number of keys following the one removed. (use range erase if possible)*
    size_type erase(const key_type& k) {
        const iterator i = find(k);
        return (i == end()) ? 0 : (erase(i), 1);
    }

    /// Remove all items in the specified range.
    /// All iterators are invalidated
    /// Linear complexity on the number of keys following the last one removed.
    void erase(iterator first, iterator last);

    /// Swap all contents with another instance
    void swap(TinyMap& other) {
        std::swap(keys, other.keys);
        std::swap(values, other.values);
        std::swap(comp, other.comp);
        return *this;
    }

    /// Remove all contents
    void clear() {
        keys.clear();
        values.clear();
    }

    /// Find the specified key, returning a value-mutable reference
    iterator find(const key_type& k) { return find_eq(*this, k); }

    /// Find the specified key
    const_iterator find(const key_type& k) const { return find_eq(*this, k); }

    /// Get the number of key matches (0 or 1)
    size_type count(const key_type& k) const { return (find(k) != end()) ? 1 : 0; }

private:
    /// Make an underlying value iterator from an underlying key iterator using `key[N]=value[N]`
    ValueIter keyToValueIter(KeyIter i) { return std::next(values.begin(), std::distance(keys.begin(), i)); }
    ValueCIter keyToValueIter(KeyCIter i) const { return std::next(values.begin(), std::distance(keys.begin(), i)); }

    /// Make an iterator from an underlying key iterator
    iterator makeIter(KeyIter i) { return {i, keyToValueIter(i)}; }
    const_iterator makeIter(KeyCIter i) const { return {i, keyToValueIter(i)}; }

    reverse_iterator makeIter(KeyRIter i) {
        const KeyIter& k = i.base();
        return reverse_iterator{iterator{k, keyToValueIter(k)}};
    }
    const_reverse_iterator makeIter(KeyCRIter i) const {
        const KeyCIter& k = i.base();
        return const_reverse_iterator{const_iterator{k, keyToValueIter(k)}};
    }

    std::vector<K> keys;
    std::vector<V> values;
    detail::TinyMapCompare<V, C> comp;
};

template <typename K, typename V, bool Sorted, typename C>
struct TinyMap<K, V, Sorted, C>::iterator
    : std::iterator<std::bidirectional_iterator_tag,
                    /*value_type=*/typename KeyValueVec::value_type,
                    /*difference_type=*/typename KeyValueVec::iterator::difference_type,
                    /*pointer_type=*/const std::pair<std::reference_wrapper<K>, std::reference_wrapper<V>>*,
                    /*reference_type=*/std::pair<K&, V&>> {
    iterator(KeyIter k, ValueIter v)
        : key(k),
          value(v) {}
    iterator(const iterator&) = default;

    std::pair<K&, V&> operator*() { return {*key, *value}; }

    // To match the `std::map` API, we need to return a pointer to a pair containing
    // the key and value, which can be used to modify the value and doesn't make a copy
    // of either.  These are temporary, and neither the result nor the references it
    // contains should be retained.
    const RefPair* operator->() { return &ref.emplace(RefPair{*key, *value}); }

    bool operator==(const iterator& rhs) const { return key == rhs.key; }
    bool operator!=(const iterator& rhs) const { return key != rhs.key; }
    iterator& operator++() noexcept {
        ++key;
        ++value;
        return *this;
    }
    iterator operator++(int) noexcept {
        const auto tmp = iterator{*this};
        operator++();
        return tmp;
    }
    iterator& operator--() noexcept {
        --key;
        --value;
        return *this;
    }
    iterator operator--(int) noexcept {
        const auto tmp = iterator{*this};
        operator--();
        return tmp;
    }

    const KeyIter& keyIter() const { return key; }

private:
    KeyIter key;
    ValueIter value;
    std::optional<RefPair> ref;
};

template <typename K, typename V, bool Sorted, typename C>
struct TinyMap<K, V, Sorted, C>::const_iterator
    : std::iterator<std::bidirectional_iterator_tag,
                    /*value_type=*/typename KeyValueVec::value_type,
                    /*difference_type=*/typename KeyValueVec::const_iterator::difference_type,
                    /*pointer_type=*/const std::pair<std::reference_wrapper<const K>, std::reference_wrapper<const V>>*,
                    /*reference_type=*/std::pair<const K&, const V&>> {
    const_iterator(KeyCIter k, ValueCIter v)
        : key(k),
          value(v) {}
    const_iterator(const const_iterator&) = default;

    std::pair<const K&, const V&> operator*() { return {*key, *value}; }

    /// See `iterator::operator->`
    const CRefPair* operator->() { return &ref.emplace(CRefPair{*key, *value}); }

    bool operator==(const const_iterator& rhs) const { return key == rhs.key; }
    bool operator!=(const const_iterator& rhs) const { return key != rhs.key; }
    const_iterator& operator++() noexcept {
        ++key;
        ++value;
        return *this;
    }
    const_iterator operator++(int) noexcept {
        const auto tmp = const_iterator{*this};
        operator++();
        return tmp;
    }
    const_iterator& operator--() noexcept {
        --key;
        --value;
        return *this;
    }
    const_iterator operator--(int) noexcept {
        const auto tmp = const_iterator{*this};
        operator--();
        return tmp;
    }

    const KeyCIter& keyIter() const { return key; }

private:
    KeyCIter key;
    ValueCIter value;
    std::optional<CRefPair> ref;
};

template <typename K, typename V, bool Sorted, typename C>
template <typename T>
inline auto TinyMap<K, V, Sorted, C>::find(T& map, const key_type& k) {
    if constexpr (Sorted) {
        return std::lower_bound(map.keys.begin(), map.keys.end(), k, map.comp);
    } else {
        return std::find(map.keys.begin(), map.keys.end(), k);
    }
}

template <typename K, typename V, bool Sorted, typename C>
template <typename T>
inline auto TinyMap<K, V, Sorted, C>::find_eq(T& map, const key_type& k) {
    const auto i = find(map, k);
    return (Sorted && i != map.keys.end() && map.comp(k, *i)) ? map.end() : map.makeIter(i);
}

template <typename K, typename V, bool Sorted, typename C>
inline auto TinyMap<K, V, Sorted, C>::insert(iterator pos, const value_type& val) -> iterator {
    if (Sorted && pos.first != keys.end() && comp(*pos.first, val.first) &&
        (pos == std::prev(end()) || !comp(val, pos.second.first) && comp(pos.second.first, val))) {
        return std::make_pair(keys.insert(pos.first, val.first), keys.insert(keyToValueIter(pos.first), val.second));
    }
    return insert(val).first;
}

template <typename K, typename V, bool S, typename C>
inline auto TinyMap<K, V, S, C>::erase(iterator pos) -> iterator {
    values.erase(keyToValueIter(pos.keyIter()));
    return makeIter(keys.erase(pos.keyIter()));
}

template <typename K, typename V, bool S, typename C>
inline void TinyMap<K, V, S, C>::erase(iterator first, iterator last) {
    values.erase(keyToValueIter(first.keyIter()), keyToValueIter(last.keyIter()));
    keys.erase(first.keyIter(), last.keyIter());
}

} // namespace util
} // namespace mbgl
