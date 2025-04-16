#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace mapbox {
namespace base {

class TypeWrapper {
public:
    TypeWrapper() noexcept : storage_(nullptr, noop_deleter) {}
    TypeWrapper(TypeWrapper&&) noexcept = default;
    TypeWrapper& operator=(TypeWrapper&&) noexcept = default;

    template <typename T> // NOLINTNEXTLINE misc-forwarding-reference-overload
    TypeWrapper(T&& value) noexcept
        : storage_(new std::decay_t<T>(std::forward<T>(value)), cast_deleter<std::decay_t<T>>) {
        static_assert(!std::is_same<TypeWrapper, std::decay_t<T>>::value, "TypeWrapper must not wrap itself.");
    }

    bool has_value() const noexcept { return static_cast<bool>(storage_); }

    template <typename T>
    T& get() noexcept { // NOLINTNEXTLINE cppcoreguidelines-pro-type-reinterpret-cast
        return *reinterpret_cast<T*>(storage_.get());
    }

private:
    template <typename T>
    static void cast_deleter(void* ptr) noexcept {
        delete reinterpret_cast<T*>(ptr); // NOLINT cppcoreguidelines-pro-type-reinterpret-cast
    }
    static void noop_deleter(void*) noexcept {}

    using storage_t = std::unique_ptr<void, void (*)(void*)>;
    storage_t storage_;
};

} // namespace base
} // namespace mapbox
