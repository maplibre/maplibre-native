#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_impl.hpp>

namespace mbgl {
namespace util {

ActionJournal::ActionJournal(const Map& map, const uint32_t logFileSize, const uint32_t logFileCount)
    : impl(std::make_unique<Impl>(map, logFileSize, logFileCount)) {}

std::vector<std::string> ActionJournal::getLog() {
    return impl->getLog();
}

void ActionJournal::clearLog() {
    impl->clearLog();
}

} // namespace util
} // namespace mbgl