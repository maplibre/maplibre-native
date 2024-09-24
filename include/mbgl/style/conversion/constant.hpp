#pragma once

#include <mbgl/style/conversion.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/string.hpp>

#include <array>
#include <string>
#include <vector>

namespace mbgl {
namespace style {
namespace conversion {

template <>
struct Converter<bool> {
    std::optional<bool> operator()(const Convertible& value, Error& error) const;
};

template <>
struct Converter<float> {
    std::optional<float> operator()(const Convertible& value, Error& error) const;
};

template <>
struct Converter<std::string> {
    std::optional<std::string> operator()(const Convertible& value, Error& error) const;
};

template <class T>
struct Converter<T, typename std::enable_if_t<std::is_enum_v<T>>> {
    std::optional<T> operator()(const Convertible& value, Error& error) const;
};

template <class T>
struct Converter<std::vector<T>, typename std::enable_if_t<std::is_enum_v<T>>> {
    std::optional<std::vector<T>> operator()(const Convertible& value, Error& error) const;
};

template <>
struct Converter<Color> {
    std::optional<Color> operator()(const Convertible& value, Error& error) const;
};

template <size_t N>
struct Converter<std::array<float, N>> {
    std::optional<std::array<float, N>> operator()(const Convertible& value, Error& error) const;
};

template <>
struct Converter<std::vector<float>> {
    std::optional<std::vector<float>> operator()(const Convertible& value, Error& error) const;
};

template <size_t N>
struct Converter<std::array<double, N>> {
    std::optional<std::array<double, N>> operator()(const Convertible& value, Error& error) const;
};

template <>
struct Converter<std::vector<std::string>> {
    std::optional<std::vector<std::string>> operator()(const Convertible& value, Error& error) const;
};

} // namespace conversion
} // namespace style
} // namespace mbgl
