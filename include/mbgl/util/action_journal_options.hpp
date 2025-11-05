#pragma once

#include <string>

namespace mbgl {

namespace util {

/**
 * @brief Holds values for Action Journal options.
 */
class ActionJournalOptions {
public:
    ActionJournalOptions() = default;
    ~ActionJournalOptions() = default;

    /**
     * @brief Enable action journal event logging, defaults to false.
     *
     * @param value true to enable, false to disable.
     * @return ActionJournalOptions for chaining options together.
     */
    ActionJournalOptions& enable(bool value = true) {
        enable_ = value;
        return *this;
    }

    /**
     * @brief Gets the previously set (or default) value.
     * @return Returns ActionJournal state
     */
    bool enabled() const { return enable_; }

    /**
     * @brief Set the log files location. An umbrela `action_journal` directory
     * will be created at the given location containing all files.
     *
     * @param path Action Journal log path.
     * @return ActionJournalOptions for chaining options together.
     */
    ActionJournalOptions& withPath(const std::string& path) {
        path_ = path;
        return *this;
    }

    /**
     * @brief Gets the previously set (or default) log path.
     * @return Returns ActionJournal log path
     */
    std::string path() const { return path_; }

    /**
     * @brief Set the file size for the log files. The action journal uses a
     * rolling log with multiple files. Total log size is equal to
     * `logFileSize * logFileCount`.
     *
     * @param size Action Journal log file size.
     * @return ActionJournalOptions for chaining options together.
     */
    ActionJournalOptions& withLogFileSize(const uint32_t size) {
        logFileSize_ = size;
        return *this;
    }

    /**
     * @brief Gets the previously set (or default) log file size.
     * @return Returns ActionJournal log file size
     */
    uint32_t logFileSize() const { return logFileSize_; }

    /**
     * @brief Set the number of log files. The action journal uses a
     * rolling log with multiple files. Total log size is equal to
     * `logFileSize * logFileCount`.
     *
     * @param count Action Journal log file count.
     * @return ActionJournalOptions for chaining options together.
     */
    ActionJournalOptions& withLogFileCount(const uint32_t count) {
        logFileCount_ = count;
        return *this;
    }

    /**
     * @brief Gets the previously set (or default) number of log files.
     * @return Returns ActionJournal log file count
     */
    uint32_t logFileCount() const { return logFileCount_; }

    /**
     * @brief Set the number of seconds to wait between rendering stats reports.
     *
     * @param interval time interval in seconds.
     * @return ActionJournalOptions for chaining options together.
     */
    ActionJournalOptions& withRenderingStatsReportInterval(const uint32_t interval) {
        renderingStatsReportInterval_ = interval;
        return *this;
    }

    /**
     * @brief Gets the previously set (or default) time interval.
     * @return Returns report time interval in seconds
     */
    uint32_t renderingStatsReportInterval() const { return renderingStatsReportInterval_; }

protected:
    bool enable_ = false;
    // path of the log
    std::string path_{"/tmp/"};
    // log file size
    uint32_t logFileSize_ = 1024 * 1024;
    // number of log files (each of `logFileSize` size)
    uint32_t logFileCount_ = 5;
    // the wait time (seconds) between rendering reports
    uint32_t renderingStatsReportInterval_ = 60;
};

} // namespace util
} // namespace mbgl
