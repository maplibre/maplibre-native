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

/// A key-value map based on vectors, optimized for small collections.
/// Loosly based on Andrei Alexandrescu's `loki::AssocVector`
/// @details Similar interface to `std::map`, but not identical, e.g.:
/// - iterators are invalidated by insert and erase operations
/// - the complexity of insert/erase is O(N) not O(log N)
/// - value_type is std::pair<K, V> not std::pair<const K, V>
template <typename K, typename V, typename C = std::less<K>>
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
    using ValueRIter = typename ValueVec::reverse_iterator;
    using ValueCRIter = typename ValueVec::const_reverse_iterator;

public:
    using A = std::allocator<K>;
    using key_type = K;
    using mapped_type = V;
    using value_type = KeyValueVec::value_type;
    using key_compare = C;
    using size_type = typename KeyVec::size_type;

    struct iterator;
    struct const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit TinyMap(const key_compare& comp_ = key_compare())
        : comp(comp_) {}

    template <typename InputIterator>
    TinyMap(InputIterator firstKeyValuePair, InputIterator lastKeyValuePair, const key_compare& comp_ = key_compare())
        : comp(comp_) {
        reserve(std::distance(firstKeyValuePair, lastKeyValuePair));
        while (firstKeyValuePair != lastKeyValuePair) {
            insert(*firstKeyValuePair++);
        }
    }

    template <typename KeyInputIterator, typename ValueInputIterator>
    TinyMap(KeyInputIterator firstKey,
            KeyInputIterator lastKey,
            ValueInputIterator firstValue,
            ValueInputIterator lastValue,
            const key_compare& comp_ = key_compare())
        : comp(comp_) {
        reserve(std::distance(firstKey, lastKey));
        while (firstKey != lastKey) {
            insert(*firstKey++, *firstValue++);
        }
    }

    TinyMap(std::initializer_list<value_type> list, const key_compare& comp_ = key_compare())
        : TinyMap(list.begin(), list.end(), comp) {}

    template <typename KeyInput, typename ValueInput>
    TinyMap(std::initializer_list<KeyInput> keys,
            std::initializer_list<ValueInput> values,
            const key_compare& comp_ = key_compare())
        : TinyMap(keys.begin(), keys.end(), values.begin(), values.end(), comp) {}

    TinyMap(const TinyMap& rhs)
        : keys(rhs.keys),
          values(rhs.values),
          comp(rhs.comp) {}
    TinyMap(TinyMap&& rhs)
        : keys(std::move(rhs.keys)),
          values(std::move(rhs.values)),
          comp(std::move(rhs.comp)) {}

    TinyMap& operator=(const TinyMap& rhs) {
        TinyMap{rhs}.swap(*this);
        return *this;
    }
    TinyMap& operator=(TinyMap&& rhs) {
        TinyMap{std::move(rhs)}.swap(*this);
        return *this;
    }

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

    void reserve(std::size_t sz) {
        keys.reserve(sz);
        values.reserve(sz);
    }

    /// Look up a key, inserting the default value if it's not present
    /// Invalidates all iterators.
    mapped_type& operator[](const key_type& key) { return insert(key, mapped_type{}).first->second; }

    /// Look up and insert a copy of the specified key and value if not present.
    /// @return as `std::map::insert`
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(const value_type& kvp) { return insert(kvp.first, kvp.second); }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(value_type&& kvp) { return insert(std::move(kvp.first), std::move(kvp.second)); }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(std::pair<const key_type&, mapped_type&&> kvp) {
        return insert(kvp.first, std::move(kvp.second));
    }

    /// Look up and move-Insert if not present.
    /// @return as `std::map::insert`
    /// Invalidates all iterators.
    std::pair<iterator, bool> insert(const key_type& key, mapped_type value) {
        auto i = std::lower_bound(keys.begin(), keys.end(), key, comp);
        if (i == keys.end() || comp(key, *i)) {
            values.insert(keyToValueIter(i), std::move(value));
            return std::make_pair(makeIter(keys.insert(i, key)), true);
        } else {
            return std::make_pair(makeIter(i), false);
        }
    }

    /// Hinted insert
    /// Invalidates all iterators.
    iterator insert(iterator pos, const value_type& val);

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    /// Remove the key/value associated with the specified iterator.
    /// @return a replacement for the given iterator, all other iterators are invalidated
    iterator erase(iterator pos);

    /// Remove the matching key, if present.
    /// @return the number of keys removed (0 or 1), all iterators are invalidated
    size_type erase(const key_type& k) {
        const iterator i = find(k);
        return (i == end()) ? 0 : (erase(i), 1);
    }

    /// Remove all items in the specified range.
    /// All iterators are invalidated
    void erase(iterator first, iterator last);

    void swap(TinyMap& other) {
        std::swap(keys, other.keys);
        std::swap(values, other.values);
        std::swap(comp, other.comp);
        return *this;
    }

    void clear() {
        keys.clear();
        values.clear();
    }

    iterator find(const key_type& k) {
        const auto i = std::lower_bound(keys.begin(), keys.end(), k, comp);
        return (i != keys.end() && comp(k, *i)) ? end() : makeIter(i);
    }

    const_iterator find(const key_type& k) const {
        const auto i = std::lower_bound(keys.begin(), keys.end(), k, comp);
        return (i != keys.end() && comp(k, *i)) ? end() : makeIter(i);
    }

    size_type count(const key_type& k) const { return (keys.find(k, comp) != keys.end()) ? 1 : 0; }

    iterator lower_bound(const key_type& k) { return makeIter(std::lower_bound(keys.begin(), keys.end(), k, comp)); }

    const_iterator lower_bound(const key_type& k) const {
        return makeIter(std::lower_bound(keys.begin(), keys.end(), k, comp));
    }

    iterator upper_bound(const key_type& k) { return makeIter(std::upper_bound(keys.begin(), keys.end(), k, comp)); }

    const_iterator upper_bound(const key_type& k) const {
        return makeIter(std::upper_bound(keys.begin(), keys.end(), k, comp));
    }

    std::pair<iterator, iterator> equal_range(const key_type& k) {
        const auto i = std::equal_range(keys.begin(), keys.end(), k, comp);
        return std::make_pair(makeIter(i.first), makeIter(i.second));
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
        const auto i = std::equal_range(keys.begin(), keys.end(), k, comp);
        return std::make_pair(makeIter(i.first), makeIter(i.second));
    }

    friend bool operator==(const TinyMap& lhs, const TinyMap& rhs) {
        return lhs.keys == rhs.keys && lhs.keys == rhs.keys;
    }

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

template <typename K, typename V, typename C>
struct TinyMap<K, V, C>::iterator
    : std::iterator<std::bidirectional_iterator_tag,
                    typename KeyValueVec::iterator::value_type,
                    typename KeyValueVec::iterator::difference_type,
                    /*pointer_type=*/std::pair<std::reference_wrapper<K>, std::reference_wrapper<V>>*,
                    /*reference_type=*/std::pair<K&, V&>> {
    iterator(KeyIter k, ValueIter v)
        : key(k),
          value(v) {}

    using RefPair = std::pair<std::reference_wrapper<K>, std::reference_wrapper<V>>;
    std::pair<K&, V&> operator*() { return {*key, *value}; }
    RefPair* operator->() { return &ref.emplace(RefPair{std::ref(*key), std::ref(*value)}); }

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

template <typename K, typename V, typename C>
struct TinyMap<K, V, C>::const_iterator
    : std::iterator<std::bidirectional_iterator_tag,
                    typename KeyValueVec::const_iterator::value_type,
                    typename KeyValueVec::const_iterator::difference_type,
                    /*pointer_type=*/std::pair<std::reference_wrapper<const K>, std::reference_wrapper<const V>>*,
                    /*reference_type=*/std::pair<const K&, const V&>> {
    const_iterator(KeyCIter k, ValueCIter v)
        : key(k),
          value(v) {}

    using RefPair = std::pair<std::reference_wrapper<const K>, std::reference_wrapper<const V>>;
    std::pair<const K&, const V&> operator*() { return {*key, *value}; }
    RefPair* operator->() { return &ref.emplace(RefPair{std::cref(*key), std::cref(*value)}); }

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
    std::optional<RefPair> ref;
};

template <typename K, typename V, typename C>
TinyMap<K, V, C>::iterator TinyMap<K, V, C>::insert(iterator pos, const value_type& val) {
    if (pos.first != keys.end() && comp(*pos.first, val.first) &&
        (pos == std::prev(end()) || !comp(val, pos.second.first) && comp(pos.second.first, val))) {
        return std::make_pair(keys.insert(pos.first, val.first), keys.insert(keyToValueIter(pos.first), val.second));
    }
    return insert(val).first;
}

template <typename K, typename V, typename C>
TinyMap<K, V, C>::iterator TinyMap<K, V, C>::erase(iterator pos) {
    values.erase(keyToValueIter(pos.keyIter()));
    return makeIter(keys.erase(pos.keyIter()));
}

template <typename K, typename V, typename C>
void TinyMap<K, V, C>::erase(iterator first, iterator last) {
    values.erase(keyToValueIter(first.keyIter()), keyToValueIter(last.keyIter()));
    keys.erase(first.keyIter(), last.keyIter());
}

} // namespace util
} // namespace mbgl
