#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
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

    static StringIdentity get(std::string_view);

    static std::string get(const StringIdentity id);

    static void clear();

    static size_t size();

protected:
    StringIndexer();
    ~StringIndexer() = default;

    using MapType = std::unordered_map<std::string_view, StringIdentity>;
    using VectorType = std::vector<std::size_t>;
    using BufferType = std::vector<char>;

    static StringIndexer& instance() {
        static StringIndexer inst;
        return inst;
    }

    MapType stringToIdentity;
    VectorType identityToString;
    BufferType buffer;

    std::shared_mutex sharedMutex;
};

} // namespace mbgl
