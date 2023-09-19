#include <mbgl/util/string_indexer.hpp>

#include <cassert>
#include <memory>

namespace mbgl {

namespace {
constexpr std::string_view empty{""};
constexpr std::size_t initialCapacity = 100;
constexpr std::size_t initialBufferCapacity = initialCapacity * 32;
} // namespace

StringIndexer::StringIndexer() {
    stringToIdentity.reserve(initialCapacity);
    identityToString.reserve(initialCapacity);
    buffer.reserve(initialBufferCapacity);
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
            auto& buffer = instance().buffer;
            const auto previousCapacity = buffer.capacity();

            const StringIdentity id = identityToString.size();
            identityToString.push_back(buffer.size());
            buffer.insert(buffer.end(), string.begin(), string.end());
            buffer.push_back(0);
            if (buffer.capacity() != previousCapacity) {
                // reallocation happened
                stringToIdentity.clear();
                for (auto iIdentity = identityToString.begin(); iIdentity != identityToString.end(); ++iIdentity) {
                    stringToIdentity[std::string_view(buffer.data() + *iIdentity)] = iIdentity -
                                                                                     identityToString.begin();
                }
            } else {
                [[maybe_unused]] auto result = stringToIdentity.insert(
                    {std::string_view(buffer.data() + identityToString.back()), id});
                assert(result.second);
            }
            return id;
        } else {
            // another writer inserted into the map
            return it->second;
        }
    }
}

std::string_view StringIndexer::get(const StringIdentity id) {
    std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);

    const auto& identityToString = instance().identityToString;
    assert(id < identityToString.size());

    return id < identityToString.size() ? (instance().buffer.data() + identityToString[id]) : empty;
}

void StringIndexer::clear() {
    std::unique_lock<std::shared_mutex> writerLock(instance().sharedMutex);

    instance().stringToIdentity.clear();
    instance().identityToString.clear();
    instance().buffer.clear();
}

size_t StringIndexer::size() {
    std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);

    [[maybe_unused]] auto& stringToIdentity = instance().stringToIdentity;
    auto& identityToString = instance().identityToString;
    assert(stringToIdentity.size() == identityToString.size());

    return identityToString.size();
}

} // namespace mbgl
