#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<bool> Converter<bool>::operator()(const Convertible& value, Error& error) const {
    std::optional<bool> converted = toBool(value);
    if (!converted) {
        error.message = "value must be a boolean";
        return std::nullopt;
    }
    return converted;
}

std::optional<float> Converter<float>::operator()(const Convertible& value, Error& error) const {
    std::optional<float> converted = toNumber(value);
    if (!converted) {
        error.message = "value must be a number";
        return std::nullopt;
    }
    return converted;
}

std::optional<std::string> Converter<std::string>::operator()(const Convertible& value, Error& error) const {
    std::optional<std::string> converted = toString(value);
    if (!converted) {
        error.message = "value must be a string";
        return std::nullopt;
    }
    return converted;
}

template <class T>
std::optional<T> Converter<T, typename std::enable_if_t<std::is_enum_v<T>>>::operator()(const Convertible& value,
                                                                                        Error& error) const {
    std::optional<std::string> string = toString(value);
    if (!string) {
        error.message = "value must be a string";
        return std::nullopt;
    }

    const auto result = Enum<T>::toEnum(*string);
    if (!result) {
        error.message = "value must be a valid enumeration value";
        return std::nullopt;
    }

    return result;
}

template <class T>
auto Converter<std::vector<T>, typename std::enable_if_t<std::is_enum_v<T>>>::operator()(const Convertible& value,
                                                                                         Error& error) const
    -> std::optional<std::vector<T>> {
    if (!isArray(value)) {
        error.message = "value must be an array";
        return std::nullopt;
    }

    std::vector<T> result;
    result.reserve(arrayLength(value));

    for (std::size_t i = 0; i < arrayLength(value); ++i) {
        std::optional<T> enumItem = Converter<T>{}(arrayMember(value, i), error);
        if (!enumItem) {
            return std::nullopt;
        }
        result.push_back(*enumItem);
    }

    return result;
}

template std::optional<AlignmentType> Converter<AlignmentType>::operator()(const Convertible&, Error&) const;
template std::optional<CirclePitchScaleType> Converter<CirclePitchScaleType>::operator()(const Convertible&,
                                                                                         Error&) const;
template std::optional<HillshadeIlluminationAnchorType> Converter<HillshadeIlluminationAnchorType>::operator()(
    const Convertible&, Error&) const;
template std::optional<IconTextFitType> Converter<IconTextFitType>::operator()(const Convertible&, Error&) const;
template std::optional<LightAnchorType> Converter<LightAnchorType>::operator()(const Convertible&, Error&) const;
template std::optional<LineCapType> Converter<LineCapType>::operator()(const Convertible&, Error&) const;
template std::optional<LineJoinType> Converter<LineJoinType>::operator()(const Convertible&, Error&) const;
template std::optional<RasterResamplingType> Converter<RasterResamplingType>::operator()(const Convertible&,
                                                                                         Error&) const;
template std::optional<SymbolAnchorType> Converter<SymbolAnchorType>::operator()(const Convertible&, Error&) const;
template std::optional<SymbolPlacementType> Converter<SymbolPlacementType>::operator()(const Convertible&,
                                                                                       Error&) const;
template std::optional<SymbolZOrderType> Converter<SymbolZOrderType>::operator()(const Convertible&, Error&) const;
template std::optional<TextJustifyType> Converter<TextJustifyType>::operator()(const Convertible&, Error&) const;
template std::optional<TextTransformType> Converter<TextTransformType>::operator()(const Convertible&, Error&) const;
template std::optional<TranslateAnchorType> Converter<TranslateAnchorType>::operator()(const Convertible&,
                                                                                       Error&) const;
template std::optional<VisibilityType> Converter<VisibilityType>::operator()(const Convertible&, Error&) const;
template std::optional<std::vector<TextVariableAnchorType>> Converter<std::vector<TextVariableAnchorType>>::operator()(
    const Convertible&, Error&) const;
template std::optional<std::vector<TextWritingModeType>> Converter<std::vector<TextWritingModeType>>::operator()(
    const Convertible&, Error&) const;

std::optional<Color> Converter<Color>::operator()(const Convertible& value, Error& error) const {
    std::optional<std::string> string = toString(value);
    if (!string) {
        error.message = "value must be a string";
        return std::nullopt;
    }

    std::optional<Color> color = Color::parse(*string);
    if (!color) {
        error.message = "value must be a valid color";
        return std::nullopt;
    }

    return color;
}

std::optional<Padding> Converter<Padding>::operator()(const Convertible& value, Error& error) const {
    std::optional<Padding> result;
    if (isArray(value)) {
        if (arrayLength(value) > 0 && arrayLength(value) <= 4) {
            auto vector = Converter<std::vector<float>>{}(value, error);
            if (vector) {
                result = Padding(*vector);
            }
        }
    } else {
        std::optional<float> number = toNumber(value);
        if (number) {
            result = Padding(*number);
        }
    }

    if (!result) {
        error.message = "value must be a number or an array of numbers (between 1 and 4 elements)";
    }
    return result;
}

std::optional<VariableAnchorOffsetCollection> Converter<VariableAnchorOffsetCollection>::operator()(
    const Convertible& value, Error& error) const {
    if (!isArray(value)) {
        error.message = "value must be an array";
        return std::nullopt;
    }

    const auto arraySize = arrayLength(value);
    if (arraySize < 1 || arraySize % 2 != 0) {
        error.message = "array must contain an even number of elements";
        return std::nullopt;
    }

    std::vector<AnchorOffsetPair> collection;
    collection.reserve(arraySize / 2);
    for (size_t index = 0; index < arraySize; index += 2) {
        Convertible offsetValue = arrayMember(value, index + 1);
        std::optional<SymbolAnchorType> anchor = Converter<SymbolAnchorType>{}(arrayMember(value, index), error);

        if (!anchor) {
            error.message = "anchor must be a valid anchor value";
            return std::nullopt;
        }

        if (!isArray(offsetValue)) {
            error.message = "anchor offset must be an array";
            return std::nullopt;
        }

        if (arrayLength(offsetValue) != 2) {
            error.message = "anchor offset must have two elements";
            return std::nullopt;
        }

        std::optional<float> xOffset = toNumber(arrayMember(offsetValue, 0));
        std::optional<float> yOffset = toNumber(arrayMember(offsetValue, 1));
        if (!xOffset || !yOffset) {
            error.message = "anchor offset must have two numbers";
            return std::nullopt;
        }

        auto anchorType = anchor.value();
        auto offset = std::array<float, 2>{xOffset.value(), yOffset.value()};
        collection.push_back(AnchorOffsetPair{anchorType, offset});
    }

    return VariableAnchorOffsetCollection(std::move(collection));
}

template <size_t N>
std::optional<std::array<float, N>> Converter<std::array<float, N>>::operator()(const Convertible& value,
                                                                                Error& error) const {
    if (!isArray(value) || arrayLength(value) != N) {
        error.message = "value must be an array of " + util::toString(N) + " numbers";
        return std::nullopt;
    }

    std::array<float, N> result;
    for (size_t i = 0; i < N; i++) {
        std::optional<float> n = toNumber(arrayMember(value, i));
        if (!n) {
            error.message = "value must be an array of " + util::toString(N) + " numbers";
            return std::nullopt;
        }
        result[i] = *n;
    }
    return result;
}

template std::optional<std::array<float, 2>> Converter<std::array<float, 2>>::operator()(const Convertible&,
                                                                                         Error&) const;
template std::optional<std::array<float, 3>> Converter<std::array<float, 3>>::operator()(const Convertible&,
                                                                                         Error&) const;
template std::optional<std::array<float, 4>> Converter<std::array<float, 4>>::operator()(const Convertible&,
                                                                                         Error&) const;

template <size_t N>
std::optional<std::array<double, N>> Converter<std::array<double, N>>::operator()(const Convertible& value,
                                                                                  Error& error) const {
    if (!isArray(value) || arrayLength(value) != N) {
        error.message = "value must be an array of " + util::toString(N) + " numbers";
        return std::nullopt;
    }

    std::array<double, N> result;
    for (size_t i = 0; i < N; i++) {
        std::optional<double> n = toDouble(arrayMember(value, i));
        if (!n) {
            error.message = "value must be an array of " + util::toString(N) + " numbers";
            return std::nullopt;
        }
        result[i] = *n;
    }
    return result;
}

template std::optional<std::array<double, 3>> Converter<std::array<double, 3>>::operator()(const Convertible&,
                                                                                           Error&) const;

std::optional<std::vector<float>> Converter<std::vector<float>>::operator()(const Convertible& value,
                                                                            Error& error) const {
    if (!isArray(value)) {
        error.message = "value must be an array";
        return std::nullopt;
    }

    std::vector<float> result;
    result.reserve(arrayLength(value));

    for (std::size_t i = 0; i < arrayLength(value); ++i) {
        std::optional<float> number = toNumber(arrayMember(value, i));
        if (!number) {
            error.message = "value must be an array of numbers";
            return std::nullopt;
        }
        result.push_back(*number);
    }

    return result;
}

std::optional<std::vector<std::string>> Converter<std::vector<std::string>>::operator()(const Convertible& value,
                                                                                        Error& error) const {
    if (!isArray(value)) {
        error.message = "value must be an array";
        return std::nullopt;
    }

    std::vector<std::string> result;
    result.reserve(arrayLength(value));

    for (std::size_t i = 0; i < arrayLength(value); ++i) {
        std::optional<std::string> string = toString(arrayMember(value, i));
        if (!string) {
            error.message = "value must be an array of strings";
            return std::nullopt;
        }
        result.push_back(*string);
    }

    return result;
}

} // namespace conversion
} // namespace style
} // namespace mbgl
