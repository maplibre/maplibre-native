#pragma once

#include <mbgl/map/map.hpp>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace mbgl {

namespace util {

class ActionJournalOptions;

class ActionJournal {
public:
    ActionJournal(const Map& map, const ActionJournalOptions& options);
    ~ActionJournal() = default;

    std::vector<std::string> getLog();
    void clearLog();

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace util
} // namespace mbgl
