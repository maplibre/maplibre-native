#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <type_traits>

namespace rust {
inline namespace cxxbridge1 {
// #include "rust/cxx.h"

namespace {
template <typename T>
class impl;
} // namespace

class String;

#ifndef CXXBRIDGE1_RUST_STR
#define CXXBRIDGE1_RUST_STR
class Str final {
public:
    Str() noexcept;
    Str(const String &) noexcept;
    Str(const std::string &);
    Str(const char *);
    Str(const char *, std::size_t);

    Str &operator=(const Str &) & noexcept = default;

    explicit operator std::string() const;

    const char *data() const noexcept;
    std::size_t size() const noexcept;
    std::size_t length() const noexcept;
    bool empty() const noexcept;

    Str(const Str &) noexcept = default;
    ~Str() noexcept = default;

    using iterator = const char *;
    using const_iterator = const char *;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

    bool operator==(const Str &) const noexcept;
    bool operator!=(const Str &) const noexcept;
    bool operator<(const Str &) const noexcept;
    bool operator<=(const Str &) const noexcept;
    bool operator>(const Str &) const noexcept;
    bool operator>=(const Str &) const noexcept;

    void swap(Str &) noexcept;

private:
    class uninit;
    Str(uninit) noexcept;
    friend impl<Str>;

    std::array<std::uintptr_t, 2> repr;
};
#endif // CXXBRIDGE1_RUST_STR
} // namespace cxxbridge1
} // namespace rust

namespace rustutils {
struct ParsedColor;
}

namespace rustutils {
#ifndef CXXBRIDGE1_STRUCT_rustutils$ParsedColor
#define CXXBRIDGE1_STRUCT_rustutils$ParsedColor
struct ParsedColor final {
    bool success;
    float r;
    float g;
    float b;
    float a;

    using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_rustutils$ParsedColor

::rustutils::ParsedColor parse_css_color(::rust::Str css_str) noexcept;
} // namespace rustutils
