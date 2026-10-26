#pragma once

#include <mln/util/immutable.hpp>

#include <memory>
#include <optional>

namespace mln {

/// A reference to the value of an `Immutable<T>` that does not keep it alive.
template <class T>
class WeakImmutable {
public:
    WeakImmutable(const Immutable<T>& immutable) noexcept
        : ptr(immutable.ptr) {}

    std::optional<Immutable<T>> lock() const noexcept {
        if (auto shared = ptr.lock()) {
            return Immutable<T>(std::move(shared));
        }
        return std::nullopt;
    }

    bool expired() const noexcept { return ptr.expired(); }

private:
    std::weak_ptr<const T> ptr;
};

} // namespace mln
