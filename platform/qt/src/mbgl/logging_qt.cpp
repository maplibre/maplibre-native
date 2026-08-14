#include <mbgl/util/logging.hpp>
#include <mbgl/util/enum.hpp>

#include <QDebug>

namespace mln {

void Log::platformRecord(EventSeverity severity, const std::string &msg) {
    qWarning() << "[" << Enum<EventSeverity>::toString(severity) << "] " << QString::fromStdString(msg);
}

} // namespace mln
