// File: src/mbgl/style/hillshade_conversions.cpp
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/enum.hpp>

namespace mbgl {
namespace style {
namespace expression {

// ValueConverter for std::vector<float>
template <>
std::optional<std::vector<float>> ValueConverter<std::vector<float>>::fromExpressionValue(const Value& value) {
    if (value.is<std::vector<Value>>()) {
        const auto& values = value.get<std::vector<Value>>();
        std::vector<float> result;
        result.reserve(values.size());
        for (const auto& v : values) {
            if (!v.is<double>()) return std::nullopt;
            result.push_back(static_cast<float>(v.get<double>()));
        }
        return result;
    }
    
    if (value.is<double>()) {
        return std::vector<float>{static_cast<float>(value.get<double>())};
    }
    
    return std::nullopt;
}

// ValueConverter for std::vector<Color>
template <>
std::optional<std::vector<Color>> ValueConverter<std::vector<Color>>::fromExpressionValue(const Value& value) {
    if (value.is<std::vector<Value>>()) {
        const auto& values = value.get<std::vector<Value>>();
        std::vector<Color> result;
        result.reserve(values.size());
        for (const auto& v : values) {
            auto color = ValueConverter<Color>::fromExpressionValue(v);
            if (!color) return std::nullopt;
            result.push_back(*color);
        }
        return result;
    }
    
    auto color = ValueConverter<Color>::fromExpressionValue(value);
    if (!color) return std::nullopt;
    return std::vector<Color>{*color};
}

// ValueConverter for HillshadeMethodType
template <>
std::optional<HillshadeMethodType> ValueConverter<HillshadeMethodType>::fromExpressionValue(const Value& value) {
    if (!value.is<std::string>()) return std::nullopt;
    
    const auto& str = value.get<std::string>();
    if (str == "standard") return HillshadeMethodType::Standard;
    if (str == "basic") return HillshadeMethodType::Basic;
    if (str == "combined") return HillshadeMethodType::Combined;
    if (str == "igor") return HillshadeMethodType::Igor;
    if (str == "multidirectional") return HillshadeMethodType::Multidirectional;
    
    return std::nullopt;
}

} // namespace expression
} // namespace style

// Enum toString for HillshadeMethodType
template <>
const char* Enum<style::HillshadeMethodType>::toString(style::HillshadeMethodType t) {
    switch (t) {
        case style::HillshadeMethodType::Standard: return "standard";
        case style::HillshadeMethodType::Basic: return "basic";
        case style::HillshadeMethodType::Combined: return "combined";
        case style::HillshadeMethodType::Igor: return "igor";
        case style::HillshadeMethodType::Multidirectional: return "multidirectional";
    }
    return "standard";
}

template <>
std::optional<style::HillshadeMethodType> Enum<style::HillshadeMethodType>::toEnum(const std::string& s) {
    if (s == "standard") return style::HillshadeMethodType::Standard;
    if (s == "basic") return style::HillshadeMethodType::Basic;
    if (s == "combined") return style::HillshadeMethodType::Combined;
    if (s == "igor") return style::HillshadeMethodType::Igor;
    if (s == "multidirectional") return style::HillshadeMethodType::Multidirectional;
    return std::nullopt;
}

} // namespace mbgl
