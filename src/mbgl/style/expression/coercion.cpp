#include <mbgl/style/expression/coercion.hpp>
#include <mbgl/style/expression/check_subtype.hpp>
#include <mbgl/style/expression/util.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/string.hpp>

#include <algorithm>

namespace mbgl {
namespace style {
namespace expression {

using CoerceFunction = EvaluationResult (*)(const Value&);

namespace {

EvaluationResult toBoolean(const Value& v) noexcept {
    return v.match([&](double f) { return static_cast<bool>(f); },
                   [&](const std::string& s) { return s.length() > 0; },
                   [&](bool b) { return b; },
                   [&](const NullValue&) { return false; },
                   [&](const Image& i) { return i.isAvailable(); },
                   [&](const auto&) { return true; });
}

EvaluationResult toNumber(const Value& v) {
    std::optional<double> result = v.match([](NullValue) -> std::optional<double> { return 0.0; },
                                           [](const double f) -> std::optional<double> { return f; },
                                           [](const std::string& s) -> std::optional<double> {
                                               try {
                                                   return util::stof(s);
                                               } catch (...) {
                                                   return {};
                                               }
                                           },
                                           [](const auto&) { return std::optional<double>(); });
    if (!result) {
        return EvaluationError{"Could not convert " + stringify(v) + " to number."};
    }
    return *result;
}

EvaluationResult toColor(const Value& colorValue) {
    return colorValue.match(
        [&](const Color& color) -> EvaluationResult { return color; },
        [&](const std::string& colorString) -> EvaluationResult {
            const std::optional<Color> result = Color::parse(colorString);
            if (result) {
                return *result;
            } else {
                return EvaluationError{"Could not parse color from value '" + colorString + "'"};
            }
        },
        [&colorValue](const std::vector<Value>& components) -> EvaluationResult {
            const std::size_t len = components.size();
            bool isNumeric = std::ranges::all_of(components,
                                                 [](const Value& item) -> bool { return item.template is<double>(); });
            if ((len == 3 || len == 4) && isNumeric) {
                Result<Color> c = {rgba(components[0].template get<double>(),
                                        components[1].template get<double>(),
                                        components[2].template get<double>(),
                                        len == 4 ? components[3].template get<double>() : 1.0)};
                if (!c) return c.error();
                return *c;
            } else {
                return EvaluationError{"Invalid rbga value " + stringify(colorValue) +
                                       ": expected an array containing either three or four "
                                       "numeric values."};
            }
        },
        [&](const auto&) -> EvaluationResult {
            return EvaluationError{"Could not parse color from value '" + stringify(colorValue) + "'"};
        });
}

EvaluationResult toPadding(const Value& paddingValue) {
    return paddingValue.match(
        [](const Padding& padding) -> EvaluationResult { return padding; },
        [&](const std::vector<Value>& components) -> EvaluationResult {
            const std::size_t len = components.size();
            const bool isNumeric = std::all_of(components.begin(), components.end(), [](const Value& item) -> bool {
                return item.template is<double>();
            });
            if ((len >= 1 && len <= 4) && isNumeric) {
                float componentsAsFloats[4] = {0};
                for (std::size_t i = 0; i < len; i++) {
                    componentsAsFloats[i] = static_cast<float>(components[i].template get<double>());
                }
                return Padding(std::span<float>(componentsAsFloats, len));
            } else {
                return EvaluationError{"Invalid padding value " + stringify(paddingValue) +
                                       ": expected an array containing from one to four "
                                       "numeric values."};
            }
        },
        [](const double number) -> EvaluationResult { return Padding(static_cast<float>(number)); },
        [&](const auto&) -> EvaluationResult {
            return EvaluationError{"Could not parse padding from value '" + stringify(paddingValue) + "'"};
        });
}

EvaluationResult toVariableAnchorOffset(const Value& value) {
    return value.match(
        [&](const VariableAnchorOffsetCollection& anchorOffset) -> EvaluationResult { return anchorOffset; },
        [&](const std::vector<Value>& components) -> EvaluationResult {
            if (components.size() % 2 != 0) {
                return EvaluationError{"Invalid variableAnchorOffset value " + stringify(components) +
                                       ": expected an array containing an even number of values."};
            }

            std::vector<AnchorOffsetPair> anchorOffsets;
            for (std::size_t i = 0; i < components.size(); i += 2) {
                const auto& anchorTypeValue = components[i];
                const auto& offsetArray = components[i + 1];

                if (!anchorTypeValue.is<std::string>() || !offsetArray.is<std::vector<Value>>() ||
                    offsetArray.get<std::vector<Value>>().size() != 2) {
                    return EvaluationError{"Invalid variableAnchorOffset value " + stringify(components) +
                                           ": expected a string value as the first and an array with two elements as "
                                           "the second element of each pair."};
                }

                const auto anchor = Enum<SymbolAnchorType>::toEnum(anchorTypeValue.get<std::string>());
                if (!anchor) {
                    return EvaluationError{"Invalid variableAnchorOffsetCollection value " + stringify(components) +
                                           ": unknown anchor type '" + anchorTypeValue.get<std::string>() + "'"};
                }

                const auto& offset = offsetArray.get<std::vector<Value>>();
                const auto x = static_cast<float>(offset[0].template get<double>());
                const auto y = static_cast<float>(offset[1].template get<double>());

                anchorOffsets.emplace_back(*anchor, std::array<float, 2>{{x, y}});
            }

            return VariableAnchorOffsetCollection(std::move(anchorOffsets));
        },
        [&](const auto&) -> EvaluationResult {
            return EvaluationError{"Could not parse variableAnchorOffset from '" + stringify(value) + "'"};
        });
}

EvaluationResult toFormatted(const Value& formattedValue) {
    return Formatted(toString(formattedValue).c_str());
}

EvaluationResult toImage(const Value& imageValue) {
    return Image(toString(imageValue).c_str());
}

EvaluationResult toStringValue(const Value& imageValue) {
    return toString(imageValue);
}

CoerceFunction getCoerceFunction(const type::Type& t) {
    if (t.is<type::BooleanType>()) {
        return toBoolean;
    } else if (t.is<type::ColorType>()) {
        return toColor;
    } else if (t.is<type::PaddingType>()) {
        return toPadding;
    } else if (t.is<type::VariableAnchorOffsetCollectionType>()) {
        return toVariableAnchorOffset;
    } else if (t.is<type::NumberType>()) {
        return toNumber;
    } else if (t.is<type::StringType>()) {
        return toStringValue;
    } else if (t.is<type::FormattedType>()) {
        return toFormatted;
    } else if (t.is<type::ImageType>()) {
        return toImage;
    }
    assert(false);
    return toStringValue;
}

/// `isRuntimeConstant` does not consider values coerced to `Type` image to be of `Kind` image, so we don't either.
constexpr Dependency extraDependency(const type::Type& /*t*/) {
    return /*t.is<type::ImageType>() ? Dependency::Image :*/ Dependency::None;
}

} // namespace

Coercion::Coercion(const type::Type& type_, std::vector<std::unique_ptr<Expression>> inputs_)
    : Expression(Kind::Coercion, type_, collectDependencies(inputs_) | extraDependency(type_)),
      coerceSingleValue(getCoerceFunction(getType())),
      inputs(std::move(inputs_)) {
    assert(!inputs.empty());
}

mbgl::Value Coercion::serialize() const {
    if (getType().is<type::FormattedType>()) {
        // Since there's no explicit "to-formatted" coercion, the only coercions
        // should be created by string expressions that get implicitly coerced
        // to "formatted".
        std::vector<mbgl::Value> serialized{{std::string("format")}};
        serialized.push_back(inputs[0]->serialize());
        serialized.emplace_back(std::unordered_map<std::string, mbgl::Value>());
        return serialized;
    } else if (getType().is<type::ImageType>()) {
        return std::vector<mbgl::Value>{{std::string("image")}, inputs[0]->serialize()};
    } else {
        return Expression::serialize();
    }
};

std::string Coercion::getOperator() const {
    auto s = getType().match(
        [](const type::BooleanType&) -> std::string_view { return "to-boolean"; },
        [](const type::ColorType&) -> std::string_view { return "to-color"; },
        [](const type::PaddingType&) -> std::string_view { return "to-padding"; },
        [](const type::NumberType&) -> std::string_view { return "to-number"; },
        [](const type::StringType&) -> std::string_view { return "to-string"; },
        [](const type::VariableAnchorOffsetCollectionType&) -> std::string_view { return "to-variableanchoroffset"; },
        [](const auto&) noexcept -> std::string_view {
            assert(false);
            return "";
        });
    return std::string(s);
}

using namespace mbgl::style::conversion;
ParseResult Coercion::parse(const Convertible& value, ParsingContext& ctx) {
    static std::unordered_map<std::string, type::Type> types{
        {"to-boolean", type::Boolean},
        {"to-color", type::Color},
        {"to-padding", type::Padding},
        {"to-number", type::Number},
        {"to-string", type::String},
        {"to-variableanchoroffset", type::VariableAnchorOffsetCollection}};

    std::size_t length = arrayLength(value);

    if (length < 2) {
        ctx.error("Expected at least one argument.");
        return ParseResult();
    }

    auto it = types.find(*toString(arrayMember(value, 0)));
    assert(it != types.end());

    if ((it->second == type::Boolean || it->second == type::String || it->second == type::Formatted ||
         it->second == type::Image) &&
        length != 2) {
        ctx.error("Expected one argument.");
        return ParseResult();
    }

    /**
     * Special form for error-coalescing coercion expressions "to-number",
     * "to-color", "to-padding".  Since these coercions can fail at runtime,
     * they accept multiple arguments, only evaluating one at a time until
     * one succeeds.
     */

    std::vector<std::unique_ptr<Expression>> parsed;
    parsed.reserve(length - 1);
    for (std::size_t i = 1; i < length; i++) {
        ParseResult input = ctx.parse(arrayMember(value, i), i, {type::Value});
        if (!input) return ParseResult();
        parsed.push_back(std::move(*input));
    }

    return ParseResult(std::make_unique<Coercion>(it->second, std::move(parsed)));
}

EvaluationResult Coercion::evaluate(const EvaluationContext& params) const {
    for (std::size_t i = 0; i < inputs.size(); i++) {
        EvaluationResult value = inputs[i]->evaluate(params);
        if (!value) return value;
        EvaluationResult coerced = coerceSingleValue(*value);
        if (coerced || i == inputs.size() - 1) {
            return coerced;
        }
    }

    assert(false);
    return EvaluationError{"Unreachable"};
};

void Coercion::eachChild(const std::function<void(const Expression&)>& visit) const {
    for (const std::unique_ptr<Expression>& input : inputs) {
        visit(*input);
    }
};

bool Coercion::operator==(const Expression& e) const noexcept {
    if (e.getKind() == Kind::Coercion) {
        const auto* rhs = static_cast<const Coercion*>(&e);
        return getType() == rhs->getType() && Expression::childrenEqual(inputs, rhs->inputs);
    }
    return false;
}

std::vector<std::optional<Value>> Coercion::possibleOutputs() const {
    std::vector<std::optional<Value>> result;
    for (const auto& input : inputs) {
        for (auto& output : input->possibleOutputs()) {
            result.push_back(std::move(output));
        }
    }
    return result;
}

} // namespace expression
} // namespace style
} // namespace mbgl
