#include <mbgl/util/string_indexer.hpp>

#include <cassert>

namespace mbgl {

namespace {
const std::string empty;
}

StringIdentity StringIndexer::get(const std::string& string) {
    {
        std::shared_lock<std::shared_mutex> readerLock(instance().sharedMutex);
        
        auto& stringToIdentity = instance().stringToIdentity;
        auto& identityToString = instance().identityToString;
        assert(stringToIdentity.size() == identityToString.size());
        
        if (auto it = stringToIdentity.find(string); it != stringToIdentity.end()) {
            return it->second;
        }
    }
    
    {
        std::unique_lock<std::shared_mutex> writerLock(instance().sharedMutex);

        auto& stringToIdentity = instance().stringToIdentity;
        auto& identityToString = instance().identityToString;
        assert(stringToIdentity.size() == identityToString.size());
        
        StringIdentity id = identityToString.size();
        auto result = stringToIdentity.insert({string, id});
        if(result.second) {
            // this writer made the insert
            identityToString.push_back(string);
        } else {
            // another writer inserted into the map
            id = result.first->second;
        }
        return id;
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
