#pragma once

#include <mbgl/map/map.hpp>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace mbgl {

namespace util {

class ActionJournalOptions;

/**
 * @brief Logs map events in a persistent rolling file format.
 * Each event is stored as a serialized json object with the event data.
 * Example
 *  {
 *      "name" : "onTileAction",
 *      "time" : "2025-04-17T13:13:13.974Z",
 *      "styleName" : "Streets",
 *      "styleURL" : "maptiler://maps/streets",
 *      "event" : {
 *          "action" : "RequestedFromNetwork",
 *          "tileX" : 0,
 *          "tileY" : 0,
 *          "tileZ" : 0,
 *          "overscaledZ" : 0,
 *          "sourceID" : "openmaptiles"
 *      }
 * }
 */
class ActionJournal {
public:
    ActionJournal(const Map& map, const ActionJournalOptions& options);
    ~ActionJournal();

    /**
     * @brief Get the action journal umbrella directory containing all logs.
     * @return log directory
     */
    std::string getLogDirectory() const;

    /**
     * @brief Get the action journal log files.
     * @return list of log files
     */
    std::vector<std::string> getLogFiles() const;

    /**
     * @brief Get the action journal events from oldest to newest.
     * Each element contains a serialized json object with the event data.
     * @return List of serialized events
     */
    std::vector<std::string> getLog();

    /**
     * @brief Clear stored action journal events by removing logs files.
     */
    void clearLog();

    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace util
} // namespace mbgl
