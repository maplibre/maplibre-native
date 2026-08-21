#include <mln/util/instrumentation.hpp>

namespace mln::instrumentation {

void setThreadName([[maybe_unused]] const std::string &name) {
#ifdef MLN_TRACY_ENABLE
    tracy::SetThreadName(name.c_str());
#endif
}

}; // namespace mln::instrumentation
