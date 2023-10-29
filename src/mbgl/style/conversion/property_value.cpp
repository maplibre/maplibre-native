#include <mbgl/style/conversion/position.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion/rotation.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <class T>
std::optional<PropertyValue<T>> Converter<PropertyValue<T>>::operator()(const Convertible& value,
                                                                        Error& error,
                                                                        bool allowDataExpressions,
                                                                        bool convertTokens) const {
    using namespace mbgl::style::expression;

    if (isUndefined(value)) {
        return PropertyValue<T>();
    }

    std::optional<PropertyExpression<T>> expression;

    if (isExpression(value)) {
        ParsingContext ctx(valueTypeToExpressionType<T>());
        ParseResult parsed = ctx.parseLayerPropertyExpression(value);
        if (!parsed) {
            error.message = ctx.getCombinedErrors();
            return std::nullopt;
        }
        expression = PropertyExpression<T>(std::move(*parsed));
    } else if (isObject(value)) {
        expression = convertFunctionToExpression<T>(value, error, convertTokens);
    } else {
        std::optional<T> constant = convert<T>(value, error);
        if (!constant) {
            return std::nullopt;
        }
        return convertTokens ? maybeConvertTokens(*constant) : PropertyValue<T>(*constant);
    }

    if (!expression) {
        return std::nullopt;
    } else if (!allowDataExpressions && !(*expression).isFeatureConstant()) {
        error.message = "data expressions not supported";
        return std::nullopt;
    } else if (!(*expression).isFeatureConstant() || !(*expression).isZoomConstant() ||
               !(*expression).isRuntimeConstant()) {
        return {std::move(*expression)};
    } else if ((*expression).getExpression().getKind() == Kind::Literal) {
        std::optional<T> constant = fromExpressionValue<T>(
            static_cast<const Literal&>((*expression).getExpression()).getValue());
        if (!constant) {
            return std::nullopt;
        }
        return PropertyValue<T>(*constant);
    } else {
        assert(false);
        error.message = "expected a literal expression";
        return std::nullopt;
    }
}

template std::optional<PropertyValue<bool>> Converter<PropertyValue<bool>>::operator()(conversion::Convertible const&,
                                                                                       conversion::Error&,
                                                                                       bool,
                                                                                       bool) const;
template std::optional<PropertyValue<float>> Converter<PropertyValue<float>>::operator()(conversion::Convertible const&,
                                                                                         conversion::Error&,
                                                                                         bool,
                                                                                         bool) const;
template std::optional<PropertyValue<std::array<float, 2>>> Converter<PropertyValue<std::array<float, 2>>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<std::array<float, 4>>> Converter<PropertyValue<std::array<float, 4>>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<std::vector<float>>> Converter<PropertyValue<std::vector<float>>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<Color>> Converter<PropertyValue<Color>>::operator()(conversion::Convertible const&,
                                                                                         conversion::Error&,
                                                                                         bool,
                                                                                         bool) const;
template std::optional<PropertyValue<std::string>> Converter<PropertyValue<std::string>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<std::vector<std::string>>>
Converter<PropertyValue<std::vector<std::string>>>::operator()(conversion::Convertible const&,
                                                               conversion::Error&,
                                                               bool,
                                                               bool) const;
template std::optional<PropertyValue<AlignmentType>> Converter<PropertyValue<AlignmentType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<CirclePitchScaleType>> Converter<PropertyValue<CirclePitchScaleType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<HillshadeIlluminationAnchorType>>
Converter<PropertyValue<HillshadeIlluminationAnchorType>>::operator()(conversion::Convertible const&,
                                                                      conversion::Error&,
                                                                      bool,
                                                                      bool) const;
template std::optional<PropertyValue<IconTextFitType>> Converter<PropertyValue<IconTextFitType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<LightAnchorType>> Converter<PropertyValue<LightAnchorType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<LineCapType>> Converter<PropertyValue<LineCapType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<LineJoinType>> Converter<PropertyValue<LineJoinType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<Position>> Converter<PropertyValue<Position>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<RasterResamplingType>> Converter<PropertyValue<RasterResamplingType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<SymbolAnchorType>> Converter<PropertyValue<SymbolAnchorType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<std::vector<TextVariableAnchorType>>>
Converter<PropertyValue<std::vector<TextVariableAnchorType>>>::operator()(conversion::Convertible const&,
                                                                          conversion::Error&,
                                                                          bool,
                                                                          bool) const;
template std::optional<PropertyValue<SymbolPlacementType>> Converter<PropertyValue<SymbolPlacementType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<SymbolZOrderType>> Converter<PropertyValue<SymbolZOrderType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<TextJustifyType>> Converter<PropertyValue<TextJustifyType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<TextTransformType>> Converter<PropertyValue<TextTransformType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<TranslateAnchorType>> Converter<PropertyValue<TranslateAnchorType>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;
template std::optional<PropertyValue<mbgl::style::expression::Formatted>>
Converter<PropertyValue<mbgl::style::expression::Formatted>>::operator()(conversion::Convertible const&,
                                                                         conversion::Error&,
                                                                         bool,
                                                                         bool) const;
template std::optional<PropertyValue<std::vector<TextWritingModeType>>>
Converter<PropertyValue<std::vector<TextWritingModeType>>>::operator()(conversion::Convertible const&,
                                                                       conversion::Error&,
                                                                       bool,
                                                                       bool) const;
template std::optional<PropertyValue<mbgl::style::expression::Image>>
Converter<PropertyValue<mbgl::style::expression::Image>>::operator()(conversion::Convertible const&,
                                                                     conversion::Error&,
                                                                     bool,
                                                                     bool) const;

std::optional<PropertyValue<std::array<double, 3>>>
mbgl::style::conversion::Converter<PropertyValue<std::array<double, 3>>, void>::operator()(const Convertible& value,
                                                                                           Error& error,
                                                                                           bool,
                                                                                           bool) const {
    std::optional<std::array<double, 3>> a = convert<std::array<double, 3>>(value, error);

    if (!a) {
        return std::nullopt;
    }
    std::array<double, 3> res;
    res[0] = (*a)[0];
    res[1] = (*a)[1];
    res[2] = (*a)[2];

    PropertyValue<std::array<double, 3>> r(res);
    return r;
}

template std::optional<PropertyValue<Rotation>> Converter<PropertyValue<Rotation>>::operator()(
    conversion::Convertible const&, conversion::Error&, bool, bool) const;

} // namespace conversion
} // namespace style
} // namespace mbgl
