#include <mbgl/util/string_indexer.hpp>

#include <cassert>

namespace mbgl {

namespace {
const std::string empty;
constexpr std::size_t initialCapacity = 100;
} // namespace

StringIndexer::StringIndexer() {
    stringToIdentity.reserve(initialCapacity);
    identityToString.reserve(initialCapacity);
}

StringIdentity StringIndexer::get(std::string_view string) {
    {
        std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);

        const auto& stringToIdentity = instance().stringToIdentity;
        [[maybe_unused]] const auto& identityToString = instance().identityToString;
        assert(stringToIdentity.size() == identityToString.size());

        if (const auto it = stringToIdentity.find(string); it != stringToIdentity.end()) {
            return it->second;
        }
    }

    {
        std::unique_lock<std::shared_mutex> writerLock(instance().sharedMutex);

        auto& stringToIdentity = instance().stringToIdentity;
        auto& identityToString = instance().identityToString;
        assert(stringToIdentity.size() == identityToString.size());

        if (const auto it = stringToIdentity.find(string); it == stringToIdentity.end()) {
            // this writer to insert
            const StringIdentity id = identityToString.size();
            identityToString.push_back(std::string(string));

            auto result = stringToIdentity.insert(
                {std::string_view(identityToString.back().data(), identityToString.back().length()), id});
            assert(result.second);
            
            return id;
        } else {
            // another writer inserted into the map
            return it->second;
        }
    }
}

const std::string& StringIndexer::get(const StringIdentity id) {
    std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);

    const auto& identityToString = instance().identityToString;
    assert(id < identityToString.size());

    return id < identityToString.size() ? identityToString[id] : empty;
}

void StringIndexer::clear() {
    std::unique_lock<std::shared_mutex> writerLock(instance().sharedMutex);

    instance().stringToIdentity.clear();
    instance().identityToString.clear();
}

size_t StringIndexer::size() {
    std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);

    [[maybe_unused]] auto& stringToIdentity = instance().stringToIdentity;
    auto& identityToString = instance().identityToString;
    assert(stringToIdentity.size() == identityToString.size());

    return identityToString.size();
}

} // namespace mbgl
