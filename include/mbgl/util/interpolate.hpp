#pragma once

#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/position.hpp>
#include <mbgl/style/rotation.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/style/variable_anchor_offset_collection.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/range.hpp>
#include <mbgl/util/string.hpp>

#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>

namespace mbgl {
namespace util {

float interpolationFactor(float base, Range<float> range, float z) noexcept;

template <class T, class Enabled = void>
struct Interpolator;

template <typename T>
T interpolate(const T& a, const T& b, const double t) {
    return Interpolator<T>()(a, b, t);
}

template <typename T>
T interpolate(const T& a, const T& b, const float t) {
    return Interpolator<T>()(a, b, t);
}

template <class T, class Enabled>
struct Interpolator {
    T operator()(const T& a, const T& b, const double t) const { return a * (1.0 - t) + b * t; }
};

template <>
struct Interpolator<float> {
    float operator()(const float& a, const float& b, const float t) const noexcept { return a * (1.0f - t) + b * t; }

    float operator()(const float& a, const float& b, const double t) const noexcept {
        return static_cast<float>(a * (1.0 - t) + b * t);
    }
};

template <class T, std::size_t N>
struct Interpolator<std::array<T, N>> {
private:
    using Array = std::array<T, N>;

    template <std::size_t... I>
    Array operator()(const Array& a, const Array& b, const double t, std::index_sequence<I...>) noexcept {
        return {{interpolate(a[I], b[I], t)...}};
    }

public:
    Array operator()(const Array& a, const Array& b, const double t) noexcept {
        return operator()(a, b, t, std::make_index_sequence<N>());
    }
};

template <std::size_t N>
struct Interpolator<std::array<float, N>> {
private:
    using Array = std::array<float, N>;

    template <std::size_t... I>
    Array operator()(const Array& a, const Array& b, const float t, std::index_sequence<I...>) noexcept {
        return {{interpolate(a[I], b[I], t)...}};
    }

public:
    Array operator()(const Array& a, const Array& b, const float t) noexcept {
        return operator()(a, b, t, std::make_index_sequence<N>());
    }
};

/// In order to accept Array<Number, N> as an output value for Curve
/// expressions, we need to have an interpolatable std::vector type.
/// However, style properties like line-dasharray are represented using
/// std::vector<float>, and should NOT be considered interpolatable.
/// So, we use std::vector<Value> to represent expression array values,
/// asserting that (a) the vectors are the same size, and (b) they contain
/// only numeric values.  (These invariants should be relatively safe,
/// being enforced by the expression type system.)
template <>
struct Interpolator<std::vector<style::expression::Value>> {
    std::vector<style::expression::Value> operator()(const std::vector<style::expression::Value>& a,
                                                     const std::vector<style::expression::Value>& b,
                                                     const double t) const {
        assert(a.size() == b.size());
        if (a.empty()) return {};
        std::vector<style::expression::Value> result;
        for (std::size_t i = 0; i < a.size(); i++) {
            assert(a[i].template is<double>());
            assert(b[i].template is<double>());
            style::expression::Value item = interpolate(a[i].template get<double>(), b[i].template get<double>(), t);
            result.push_back(item);
        }
        return result;
    }
};

template <>
struct Interpolator<style::Position> {
public:
    style::Position operator()(const style::Position& a, const style::Position& b, const float t) noexcept {
        auto pos = style::Position();
        auto interpolated = interpolate(a.getCartesian(), b.getCartesian(), t);
        pos.setCartesian(interpolated);
        return {pos};
    }
};

template <>
struct Interpolator<Color> {
public:
    Color operator()(const Color& a, const Color& b, const float t) noexcept {
        return {interpolate(a.r, b.r, t), interpolate(a.g, b.g, t), interpolate(a.b, b.b, t), interpolate(a.a, b.a, t)};
    }

    Color operator()(const Color& a, const Color& b, const double t) noexcept {
        return {interpolate(a.r, b.r, t), interpolate(a.g, b.g, t), interpolate(a.b, b.b, t), interpolate(a.a, b.a, t)};
    }
};

template <>
struct Interpolator<Padding> {
public:
    Padding operator()(const Padding& a, const Padding& b, const float t) const noexcept {
        return {interpolate(a.top, b.top, t),
                interpolate(a.right, b.right, t),
                interpolate(a.bottom, b.bottom, t),
                interpolate(a.left, b.left, t)};
    }

    Padding operator()(const Padding& a, const Padding& b, const double t) const noexcept {
        return {interpolate(a.top, b.top, t),
                interpolate(a.right, b.right, t),
                interpolate(a.bottom, b.bottom, t),
                interpolate(a.left, b.left, t)};
    }
};

template <>
struct Interpolator<VariableAnchorOffsetCollection> {
public:
    VariableAnchorOffsetCollection operator()(const VariableAnchorOffsetCollection& a,
                                              const VariableAnchorOffsetCollection& b,
                                              const float t) const {
        if (a.size() != b.size()) {
            throw std::runtime_error("Cannot interpolate values of different length. from: " + a.toString() +
                                     ", to: " + b.toString());
        }
        std::vector<AnchorOffsetPair> offsetMap;
        offsetMap.reserve(a.size());
        for (size_t index = 0; index < a.size(); index++) {
            const auto& aPair = a[index];
            const auto& bPair = b[index];
            if (aPair.anchorType != bPair.anchorType) {
                throw std::runtime_error(
                    "Cannot interpolate values containing mismatched anchors. index: " + util::toString(index) +
                    "from: " + Enum<style::SymbolAnchorType>::toString(aPair.anchorType) +
                    ", to: " + Enum<style::SymbolAnchorType>::toString(bPair.anchorType));
            }
            auto offset = std::array<float, 2>{interpolate(aPair.offset[0], bPair.offset[0], t),
                                               interpolate(aPair.offset[1], bPair.offset[1], t)};
            offsetMap.emplace_back(aPair.anchorType, offset);
        }

        return VariableAnchorOffsetCollection(std::move(offsetMap));
    }
};

template <>
struct Interpolator<style::Rotation> {
public:
    style::Rotation operator()(const style::Rotation& a, const style::Rotation& b, const double t) noexcept {
        assert(a.period() == b.period());
        auto period = a.period();
        auto aAngle = std::fmod(a.getAngle(), period);
        auto bAngle = std::fmod(b.getAngle(), period);

        if (aAngle - bAngle > period * 0.5) {
            return {std::fmod(aAngle * (1.0 - t) + (bAngle + period) * t, period)};
        }

        if (aAngle - bAngle < period * -0.5) {
            return {std::fmod((aAngle + period) * (1.0 - t) + bAngle * t, period)};
        }

        return {aAngle * (1.0 - t) + bAngle * t};
    }
};

struct Uninterpolated {
    template <class T>
    T operator()(const T& a, const T&, const double) const {
        return a;
    }
};

template <>
struct Interpolator<bool> : Uninterpolated {};

template <class T>
struct Interpolator<T, typename std::enable_if_t<std::is_enum_v<T>>> : Uninterpolated {};

template <>
struct Interpolator<std::string> : Uninterpolated {};

template <class T>
struct Interpolator<std::vector<T>> : Uninterpolated {};

template <class T>
struct Interpolatable
    : std::conditional_t<!std::is_base_of_v<Uninterpolated, Interpolator<T>>, std::true_type, std::false_type> {};

} // namespace util
} // namespace mbgl
