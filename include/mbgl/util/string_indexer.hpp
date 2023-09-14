#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

namespace mbgl {

using StringIdentity = size_t;

class StringIndexer {
public:
    StringIndexer(StringIndexer const&) = delete;
    StringIndexer(StringIndexer&&) = delete;
    void operator=(StringIndexer const&) = delete;

    static StringIdentity get(const std::string& string);

    static const std::string& get(const StringIdentity id);

    static void clear();
    
    static size_t size();

protected:
    StringIndexer() = default;
    ~StringIndexer() = default;

    using MapType = std::unordered_map<std::string, StringIdentity>;
    using VectorType = std::vector<std::string>;

    static StringIndexer& instance() {
        static StringIndexer inst;
        return inst;
    }
    
    MapType stringToIdentity;
    VectorType identityToString;
    std::shared_mutex sharedMutex;
};

} // namespace mbgl
