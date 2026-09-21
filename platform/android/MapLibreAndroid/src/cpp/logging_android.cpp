#include <mln/util/logging.hpp>

#include "logger.hpp"
#include "attach_env.hpp"

namespace mln {

void Log::platformRecord(EventSeverity severity, const std::string &msg) {
    auto env{android::AttachEnv()};
    android::Logger::log(*env, severity, msg);
}

} // namespace mln
