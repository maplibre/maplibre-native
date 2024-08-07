#pragma once

#include <list>
#include <stdexcept>
#include <unordered_map>

namespace mbgl {

// Simple non-thread-safe LRU cache
// Item must be hashable
template <typename Item>
class LRU {
public:
    // Number of cached items
    size_t size() const { return list.size(); }

    // Check if cache is empty
    bool empty() const { return list.empty(); }

    // Access x
    // If x is already cached, move it to the front (most recently used)
    void touch(Item x) {
        auto it = map.find(x);
        if (it != map.end()) {
            list.splice(list.begin(), list, it->second);
        } else {
            list.push_front(x);
            map[std::move(x)] = list.begin();
        }
    }

    // Evict the least recently used item
    // If the cache is empty, throw an exception
    Item evict() {
        if (list.empty()) {
            throw std::runtime_error("LRU::evict: empty cache");
        }
        Item x = std::move(list.back());
        map.erase(list.back());
        list.pop_back();
        return x;
    }

    // Check if item exists in cache without changing its order
    bool isHit(Item x) { return map.find(std::move(x)) != map.end(); }

    // remove item from cache if it exists
    void remove(Item x) {
        auto it = map.find(x);
        if (it != map.end()) {
            list.erase(it->second);
            map.erase(it);
        }
    }

private:
    std::list<Item> list;
    std::unordered_map<Item, typename std::list<Item>::iterator> map;
};

} // namespace mbgl
