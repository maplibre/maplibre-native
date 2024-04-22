#pragma once

#include <mbgl/style/expression/collator.hpp>
#include <mbgl/style/expression/formatted.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/expression/type.hpp>
#include <mbgl/style/position.hpp>
#include <mbgl/style/rotation.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/variant.hpp>

#include <array>
#include <vector>
#include <optional>

namespace mbgl {
namespace style {
namespace expression {

struct Value;

using ValueBase = variant<NullValue,
                          bool,
                          double,
                          std::string,
                          Color,
                          Collator,
                          Formatted,
                          Image,
                          mapbox::util::recursive_wrapper<std::vector<Value>>,
                          mapbox::util::recursive_wrapper<std::unordered_map<std::string, Value>>>;
struct Value : ValueBase {
    VARIANT_INLINE Value() noexcept {}

    template <typename T>
    VARIANT_INLINE Value(T&& val)
        : ValueBase(std::forward<T>(val)) {}

    template <typename T>
    VARIANT_INLINE Value(const T& val)
        : ValueBase(val) {}

    // Javascript's Number.MAX_SAFE_INTEGER
    static constexpr uint64_t maxSafeInteger() noexcept { return 9007199254740991ULL; }

    static constexpr bool isSafeInteger(uint64_t x) noexcept { return x <= maxSafeInteger(); };
    static constexpr bool isSafeInteger(int64_t x) noexcept {
        return static_cast<uint64_t>(x > 0 ? x : -x) <= maxSafeInteger();
    }
    static constexpr bool isSafeInteger(double x) noexcept {
        return static_cast<uint64_t>(x > 0 ? x : -x) <= maxSafeInteger();
    }
};

constexpr NullValue Null = NullValue();

type::Type typeOf(const Value& value);

std::string toString(const Value& value);
std::string stringify(const Value& value);

/*
  Returns a Type object representing the expression type that corresponds to
  the value type T.  (Specialized for primitives and specific array types in
  the .cpp.)
*/
template <typename T>
type::Type valueTypeToExpressionType();

/*
  Conversions between style value types and expression::Value
*/

template <class T, class Enable = void>
struct ValueConverter {
    static Value toExpressionValue(const T& value) { return Value(value); }

    static std::optional<T> fromExpressionValue(const Value& value) {
        return value.template is<T>() ? value.template get<T>() : std::optional<T>();
    }
};

template <>
struct ValueConverter<Value> {
    static type::Type expressionType() { return type::Value; }
    static Value toExpressionValue(const Value& value) { return value; }
    static std::optional<Value> fromExpressionValue(const Value& value) { return value; }
};

template <>
struct ValueConverter<mbgl::Value> {
    static Value toExpressionValue(const mbgl::Value& value);
    static mbgl::Value fromExpressionValue(const Value& value);
};

template <>
struct ValueConverter<float> {
    static type::Type expressionType() { return type::Number; }
    static Value toExpressionValue(float value);
    static std::optional<float> fromExpressionValue(const Value& value);
};

template <typename T, std::size_t N>
struct ValueConverter<std::array<T, N>> {
    static type::Type expressionType() { return type::Array(valueTypeToExpressionType<T>(), N); }
    static Value toExpressionValue(const std::array<T, N>& value);
    static std::optional<std::array<T, N>> fromExpressionValue(const Value& value);
};

template <typename T>
struct ValueConverter<std::vector<T>> {
    static type::Type expressionType() { return type::Array(valueTypeToExpressionType<T>()); }
    static Value toExpressionValue(const std::vector<T>& value);
    static std::optional<std::vector<T>> fromExpressionValue(const Value& value);
};

template <>
struct ValueConverter<Position> {
    static type::Type expressionType() { return type::Array(type::Number, 3); }
    static Value toExpressionValue(const mbgl::style::Position& value);
    static std::optional<Position> fromExpressionValue(const Value& v);
};

template <typename T>
struct ValueConverter<T, std::enable_if_t<std::is_enum_v<T>>> {
    static type::Type expressionType() { return type::String; }
    static Value toExpressionValue(const T& value);
    static std::optional<T> fromExpressionValue(const Value& value);
};

template <typename T>
Value toExpressionValue(const T& value) {
    return ValueConverter<T>::toExpressionValue(value);
}

template <typename T>
std::optional<T> fromExpressionValue(const Value& value) {
    return ValueConverter<T>::fromExpressionValue(value);
}

template <typename T>
std::vector<std::optional<T>> fromExpressionValues(const std::vector<std::optional<Value>>& values) {
    std::vector<std::optional<T>> result;
    result.reserve(values.size());
    for (const auto& value : values) {
        result.push_back(value ? fromExpressionValue<T>(*value) : std::nullopt);
    }
    return result;
}

template <>
struct ValueConverter<Rotation> {
    static type::Type expressionType() { return type::Number; }
    static Value toExpressionValue(const mbgl::style::Rotation& value);
    static std::optional<Rotation> fromExpressionValue(const Value& v);
};

} // namespace expression
} // namespace style
} // namespace mbgl
