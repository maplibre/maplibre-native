#include <mln/util/logging.hpp>
#include <mln/util/enum.hpp>

#include <iostream>

namespace mln {

void Log::platformRecord(EventSeverity severity, const std::string &msg) {
    std::cerr << "[" << Enum<EventSeverity>::toString(severity) << "] " << msg << '\n';
}

} // namespace mln
