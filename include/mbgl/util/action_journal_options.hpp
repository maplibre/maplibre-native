#pragma once

#include <string>
#include <filesystem>

namespace mbgl {

namespace util {

class ActionJournalOptions {
public:
    ActionJournalOptions(bool enable = false)
        : enable_(enable) {}
    ~ActionJournalOptions() = default;

    bool enabled() const { return enable_; }

    ActionJournalOptions& withPath(const std::string& path) {
        path_ = path;
        return *this;
    }
    ActionJournalOptions& withPath(const std::filesystem::path& path) {
        path_ = path;
        return *this;
    }
    std::filesystem::path path() { return path_; }

    ActionJournalOptions& withLogFileSize(const uint32_t size) {
        logFileSize_ = size;
        return *this;
    }
    uint32_t logFileSize() { return logFileSize_; }

    ActionJournalOptions& withLogFileCount(const uint32_t count) {
        logFileCount_ = count;
        return *this;
    }
    uint32_t logFileCount() { return logFileCount_; }

protected:
    bool enable_ = false;
    // path of the log
    std::filesystem::path path_{"/tmp/"};
    // log file size
    uint32_t logFileSize_ = 1024 * 1024;
    // number of log files (each of `logFileSize` size)
    uint32_t logFileCount_ = 5;
};

} // namespace util
} // namespace mbgl
