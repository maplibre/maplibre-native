#pragma once

#include <functional>

namespace mbgl {
namespace util {

// C++20 non-copying lambda capture, pending C++23 `move_only_function`
// Adapted from https://stackoverflow.com/a/52358928/135138
template <typename T>
class unique_function : public std::function<T> {
    using base = std::function<T>;

public:
    unique_function() noexcept = default;

    unique_function(std::nullptr_t) noexcept
        : base(nullptr) {}

    template <typename Fn>
        requires requires(Fn f) {
            { f() } -> std::same_as<std::invoke_result_t<T>>;
        }
    unique_function(Fn &&f)
        : base(wrapper<std::remove_reference_t<Fn>>{std::forward<std::remove_reference_t<Fn>>(f)}) {}

    unique_function(unique_function &&) = default;

    unique_function &operator=(unique_function &&) = default;

    template <typename Fn>
        requires requires(Fn f) {
            { f() } -> std::same_as<std::invoke_result_t<T>>;
        }
    unique_function &operator=(Fn &&f) {
        base::operator=(wrapper<std::remove_reference_t<Fn>>{std::forward<std::remove_reference_t<Fn>>(f)});
        return *this;
    }

    using base::operator();

private:
    template <typename Fn, typename En = void>
    struct wrapper;

    // specialization for MoveConstructible-only Fn
    template <typename Fn>
    struct wrapper<Fn, std::enable_if_t<!std::is_copy_constructible_v<Fn> && std::is_move_constructible_v<Fn>>> {
        Fn fn;

        wrapper(Fn &&fn_)
            : fn(std::forward<Fn>(fn_)) {}
        wrapper(wrapper &&) = default;
        wrapper &operator=(wrapper &&) = default;

        // these two functions are instantiated by std::function and are never called
        // We can't delete this or `fn` is uninitialized for non-DefaultContructible types
        wrapper(const wrapper &rhs)
            : fn(const_cast<Fn &&>(rhs.fn)) {
            throw std::runtime_error{{}};
        }
        wrapper &operator=(const wrapper &) = delete;

        template <typename... Args>
        auto operator()(Args &&...args) {
            return fn(std::forward<Args>(args)...);
        }
    };
};
} // namespace util
} // namespace mbgl
