#pragma once

namespace mbgl {
namespace util {

/// Wrapper that doesn't participate in copying.
/// This allows a default-copyable object to have members that aren't copied
template<typename T> struct SuppressCopies {
    SuppressCopies() = default;
    SuppressCopies(SuppressCopies const&) {}
    SuppressCopies(SuppressCopies&& other) : value(std::forward<T>(other.value)) {}
    SuppressCopies& operator=(SuppressCopies o) { std::swap(value, o.value); return *this; }
    operator T&() { return value; }
    T& get()      { return value; }
private:
    T value;
};

} // namespace util
} // namespace mbgl
