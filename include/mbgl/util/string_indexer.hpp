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

using StringIdentity = std::size_t;

class StringIndexer {
protected:
    StringIndexer();

public:
    StringIndexer(StringIndexer const&) = delete;
    StringIndexer(StringIndexer&&) = delete;
    void operator=(StringIndexer const&) = delete;
    ~StringIndexer() = default;

    StringIdentity get(std::string_view);

    std::string get(const StringIdentity id);

    size_t size();

protected:
    friend StringIndexer& stringIndexer();

    std::unordered_map<std::string_view, StringIdentity> stringToIdentity;
    std::vector<StringIdentity> identityToString;
    std::vector<char> buffer;

    std::shared_mutex sharedMutex;
};

/// StringIndexer singleton
StringIndexer& stringIndexer();

} // namespace mbgl
