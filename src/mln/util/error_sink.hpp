#pragma once

#include <string>

namespace mln {

class SymbolErrorObserver;

/// Thread-local destination for errors detected by code too deep to reach an observer on its own,
/// e.g. the symbol guard checks in `SymbolInstance` and `SymbolBucket` (see `MLN_SYMBOL_GUARDS`).
///
/// Each thread which owns an observer installs it with an `ErrorScope` around the work it drives,
/// so that a failure is reported as soon as it's detected, through the observer owning the thread
/// the check ran on: `TileObserver` for tile parsing/layout, `RendererObserver` for the render loop.
class ErrorSink {
public:
    /// Report an error to the observer installed on the current thread, if any. Does nothing
    /// otherwise, so check sites don't have to care whether an observer is in reach.
    static void report(const std::string& message);

private:
    friend class ErrorScope;

    /// The observer installed on the calling thread, `nullptr` when there's none.
    static SymbolErrorObserver*& current();
};

/// Installs an observer as the current thread's error destination for the lifetime of the scope,
/// restoring the previous one on destruction.
class ErrorScope {
public:
    /// @param observer May be null, which mutes reporting for the duration of the scope.
    explicit ErrorScope(SymbolErrorObserver* observer);
    ~ErrorScope();

    ErrorScope(const ErrorScope&) = delete;
    ErrorScope& operator=(const ErrorScope&) = delete;

private:
    SymbolErrorObserver* previous;
};

} // namespace mln
