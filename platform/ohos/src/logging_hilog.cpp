#include <mbgl/util/enum.hpp>
#include <mbgl/util/logging.hpp>

#include <hilog/log.h>

#include <string>

namespace mbgl {
namespace {

constexpr unsigned int kMapLibreHilogDomain = 0x4d4c;
constexpr char kMapLibreHilogTag[] = "MapLibreNative";

LogLevel logLevelForSeverity(EventSeverity severity) {
    switch (severity) {
        case EventSeverity::Debug:
            return LOG_DEBUG;
        case EventSeverity::Info:
            return LOG_INFO;
        case EventSeverity::Warning:
            return LOG_WARN;
        case EventSeverity::Error:
            return LOG_ERROR;
        case EventSeverity::SeverityCount:
            break;
    }
    return LOG_INFO;
}

} // namespace

void Log::platformRecord(EventSeverity severity, const std::string& msg) {
    const auto message = std::string("[") + Enum<EventSeverity>::toString(severity) + "] " + msg;
    OH_LOG_PrintMsg(LOG_APP, logLevelForSeverity(severity), kMapLibreHilogDomain, kMapLibreHilogTag, message.c_str());
}

} // namespace mbgl
