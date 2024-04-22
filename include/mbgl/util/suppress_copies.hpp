#pragma once

namespace mbgl {
namespace util {

/// Wrapper that doesn't participate in copying.
/// This allows a default-copyable object to have members that aren't copied
template <typename T>
struct SuppressCopies {
    SuppressCopies() = default;
    SuppressCopies(SuppressCopies const&) noexcept {}
    SuppressCopies(SuppressCopies&& other)
        : value(std::forward<T>(other.value)) noexcept {}
    SuppressCopies& operator=(SuppressCopies o) noexcept {
        std::swap(value, o.value);
        return *this;
    }
    operator T&() noexcept { return value; }
    T& get() noexcept { return value; }

private:
    T value;
};

} // namespace util
} // namespace mbgl
