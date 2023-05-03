#include <mbgl/style/conversion/color_ramp_property_value.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/style/expression/is_expression.hpp>
#include <mbgl/style/expression/parsing_context.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<ColorRampPropertyValue> Converter<ColorRampPropertyValue>::operator()(const Convertible& value,
                                                                                    Error& error,
                                                                                    bool,
                                                                                    bool) const {
    using namespace mbgl::style::expression;
    if (isUndefined(value)) {
        return ColorRampPropertyValue();
    } else if (isExpression(value)) {
        ParsingContext ctx(type::Color);
        ParseResult expression = ctx.parseLayerPropertyExpression(value);
        if (!expression) {
            error.message = ctx.getCombinedErrors();
            return std::nullopt;
        }
        assert(*expression);
        if (!isFeatureConstant(**expression)) {
            error.message = "data expressions not supported";
            return std::nullopt;
        }
        if (!isZoomConstant(**expression)) {
            error.message = "zoom expressions not supported";
            return std::nullopt;
        }
        return ColorRampPropertyValue(std::move(*expression));
    } else {
        error.message = "color ramp must be an expression";
        return std::nullopt;
    }
}

} // namespace conversion
} // namespace style
} // namespace mbgl
