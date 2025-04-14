#pragma once

#include <string>
#include <vector>
#include <memory>

namespace mbgl {

class Map;

namespace util {

class ActionJournal {
public:
    ActionJournal(const Map& map,
                  const uint32_t logFileSize = 1024 * 1024,
                  const uint32_t logFileCount = 5);
    ~ActionJournal() = default;

    std::vector<std::string> getLog();
    void clearLog();

private:
    class Impl;
    const std::unique_ptr<Impl> impl;

    friend class ActionJournalEvent;
    friend class Map;
};

} // namespace util
} // namespace mbgl
