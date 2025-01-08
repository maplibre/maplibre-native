#pragma once

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>

// Polyfill needed by Qt when building for Android with GCC
#if defined(__ANDROID__) && defined(__GLIBCXX__)

namespace std {

inline int stoi(const std::string &str) {
    return atoi(str.c_str());
}

inline float stof(const std::string &str) {
    return static_cast<float>(atof(str.c_str()));
}

} // namespace std

#endif

namespace mbgl {
namespace util {

std::string toString(int64_t);
std::string toString(uint64_t);
std::string toString(int32_t);
std::string toString(uint32_t);
std::string toString(double, bool decimal = false);

inline std::string toString(int16_t t) {
    return toString(static_cast<int32_t>(t));
}

inline std::string toString(uint16_t t) {
    return toString(static_cast<uint32_t>(t));
}

inline std::string toString(int8_t t) {
    return toString(static_cast<int32_t>(t));
}

inline std::string toString(uint8_t t) {
    return toString(static_cast<uint32_t>(t));
}

// NOLINTBEGIN(bugprone-incorrect-enable-if)

template <typename = std::enable_if<!std::is_same_v<uint64_t, unsigned long>>>
inline std::string toString(unsigned long t) {
    return toString(static_cast<uint64_t>(t));
}

template <typename = std::enable_if<!std::is_same_v<uint64_t, unsigned long long>>>
inline std::string toString(unsigned long long t) {
    return toString(static_cast<uint64_t>(t));
}

template <typename = std::enable_if<!std::is_same_v<int64_t, long>>>
inline std::string toString(long t) {
    return toString(static_cast<int64_t>(t));
}

template <typename = std::enable_if<!std::is_same_v<int64_t, long long>>>
inline std::string toString(long long t) {
    return toString(static_cast<int64_t>(t));
}

// NOLINTEND(bugprone-incorrect-enable-if)

inline std::string toString(float t, bool decimal = false) {
    return toString(static_cast<double>(t), decimal);
}

inline std::string toString(long double t, bool decimal = false) {
    return toString(static_cast<double>(t), decimal);
}

std::string toString(const std::thread::id &);

std::string toString(const std::exception_ptr &);

template <class T>
std::string toString(T) = delete;

std::string toHex(uint32_t);
std::string toHex(uint64_t);

inline float stof(const std::string &str) {
    return std::stof(str);
}

// https://gist.github.com/Baduit/63c4ea0f248451f7047c1b003c8335d5
// Note: asked author to clarify license

template <std::size_t ArraySize>
struct CompileTimeString {
    constexpr CompileTimeString() noexcept = default;

    constexpr CompileTimeString(const char (&literal)[ArraySize]) noexcept { std::ranges::copy(literal, data); }

    template <std::size_t OtherSize>
    constexpr auto operator+(const CompileTimeString<OtherSize> &other) const noexcept {
        CompileTimeString<ArraySize + OtherSize - 1> result;
        std::ranges::copy(data, result.data);
        std::ranges::copy(other.data, result.data + ArraySize - 1);
        return result;
    }

    // Don't count the \0 at the end
    constexpr std::size_t size() const { return ArraySize - 1; }

    constexpr auto begin() noexcept { return std::begin(data); }
    constexpr auto end() noexcept { return std::end(data); }
    constexpr auto cbegin() const noexcept { return std::cbegin(data); }
    constexpr auto cend() const noexcept { return std::cend(data); }
    constexpr auto rbegin() noexcept { return std::rbegin(data); }
    constexpr auto rend() noexcept { return std::rend(data); }
    constexpr auto crbegin() const noexcept { return std::crbegin(data); }
    constexpr auto crend() const noexcept { return std::crend(data); }

    template <std::size_t OtherSize>
    constexpr bool operator==(const CompileTimeString<OtherSize> &other) const {
        return as_string_view() == other.as_string_view();
    }

    constexpr bool starts_with(std::string_view sv) const noexcept { return as_string_view().starts_with(sv); }
    constexpr bool starts_with(char ch) const noexcept { return as_string_view().starts_with(ch); }
    constexpr bool starts_with(const char *s) const { return as_string_view().starts_with(s); }

    template <std::size_t OtherSize>
    constexpr bool starts_with(const CompileTimeString<OtherSize> &other) const {
        return starts_with(other.as_string_view());
    }

    // https://en.cppreference.com/w/cpp/language/reference
    // https://en.cppreference.com/w/cpp/language/member_functions#Member_functions_with_ref-qualifier
    constexpr operator std::string_view() const & { return as_string_view(); }
    constexpr operator std::string_view() const && = delete;

    constexpr std::string_view as_string_view() const & { return std::string_view(data, ArraySize - 1); }
    constexpr std::string_view as_string_view() const && = delete;

    char data[ArraySize];
};

template <CompileTimeString Str>
constexpr auto operator""_cts() {
    return Str;
}

} // namespace util
} // namespace mbgl

// Android's libstdc++ doesn't have std::to_string()
#if defined(__ANDROID__) && defined(__GLIBCXX__)

namespace std {

template <typename T>
inline std::string to_string(T value) {
    return mbgl::util::toString(value);
}

} // namespace std

#endif
