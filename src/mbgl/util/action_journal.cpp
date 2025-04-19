#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_impl.hpp>

namespace mbgl {
namespace util {

ActionJournal::ActionJournal(const Map& map, const ActionJournalOptions& options)
    : impl(std::make_unique<Impl>(map, options)) {}

std::vector<std::string> ActionJournal::getLog() {
    return impl->getLog();
}

void ActionJournal::clearLog() {
    impl->clearLog();
}

} // namespace util
} // namespace mbgl
