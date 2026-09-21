#include <mln/util/error_sink.hpp>
#include <mln/util/symbol_error_observer.hpp>

namespace mln {

SymbolErrorObserver*& ErrorSink::current() {
    static thread_local SymbolErrorObserver* observer = nullptr;
    return observer;
}

void ErrorSink::report(const std::string& message) {
    if (auto* observer = current()) {
        observer->onSymbolError(message);
    }
}

ErrorScope::ErrorScope(SymbolErrorObserver* observer)
    : previous(ErrorSink::current()) {
    ErrorSink::current() = observer;
}

ErrorScope::~ErrorScope() {
    ErrorSink::current() = previous;
}

} // namespace mln
