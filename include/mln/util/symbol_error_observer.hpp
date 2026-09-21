#pragma once

#include <string>

namespace mln {

/// Receives errors detected by the symbol guard checks (see `MLN_SYMBOL_GUARDS`).
/// Both `TileObserver` and `RendererObserver` derive from this, so that whichever of them owns
/// the thread a check ran on can be installed as that thread's error destination, see `ErrorSink`.
class SymbolErrorObserver {
public:
    virtual ~SymbolErrorObserver() = default;

    /// Called with a description of a single corrupted item, on the thread which detected it.
    /// Several calls may be made for the same bucket if it holds more than one corrupted item.
    virtual void onSymbolError(const std::string&) {}
};

} // namespace mln
