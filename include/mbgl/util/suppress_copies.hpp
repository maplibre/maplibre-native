#pragma once

namespace mbgl {
namespace util {

/// Wrapper that doesn't participate in copying.
/// This allows a default-copyable object to have members that aren't copied
template <typename T>
struct SuppressCopies {
    SuppressCopies() = default;
    SuppressCopies(SuppressCopies const&) {}
    SuppressCopies(SuppressCopies&& other)
        : value(std::forward<T>(other.value)) {}
    SuppressCopies& operator=(T&& o) {
        value = std::move(o);
        return *this;
    }
    SuppressCopies& operator=(SuppressCopies o) {
        std::swap(value, o.value);
        return *this;
    }

    T& operator->() { return value; }
    const T& operator->() const { return value; }

    operator T&() { return value; }
    T& get() { return value; }

private:
    T value;
};

} // namespace util
} // namespace mbgl
