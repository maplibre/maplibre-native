#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {

using StringIdentity = size_t;

class StringIndexer {
public:
    StringIndexer() = delete;
    StringIndexer(StringIndexer const&) = delete;
    StringIndexer(StringIndexer&&) = delete;
    void operator=(StringIndexer const&) = delete;
    ~StringIndexer() = delete;

    static StringIdentity get(const std::string& string);

    static const std::string& get(const StringIdentity id);

protected:
    using MapType = std::unordered_map<std::string, StringIdentity>;
    using VectorType = std::vector<std::string>;

    static MapType& getMap() {
        static MapType stringToIdentity;
        return stringToIdentity;
    }

    static VectorType& getVector() {
        static VectorType identityToString;
        return identityToString;
    }
};

} // namespace mbgl
