#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_impl.hpp>

namespace mbgl {
namespace util {

ActionJournal::ActionJournal(const Map& map, const ActionJournalOptions& options)
    : impl(std::make_unique<Impl>(map, options)) {
    impl->onMapCreate();
}

ActionJournal::~ActionJournal() {
    impl->onMapDestroy();
}

std::string ActionJournal::getLogDirectory() const {
    return impl->getLogDirectory();
}

std::vector<std::string> ActionJournal::getLogFiles() const {
    return impl->getLogFiles();
}

std::vector<std::string> ActionJournal::getLog() {
    return impl->getLog();
}

void ActionJournal::clearLog() {
    impl->clearLog();
}

} // namespace util
} // namespace mbgl
