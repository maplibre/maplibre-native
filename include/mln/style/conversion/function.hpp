#pragma once

#include <mln/style/property_expression.hpp>
#include <mln/style/conversion/constant.hpp>
#include <mln/style/conversion.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/value.hpp>

#include <optional>

namespace mln {
namespace style {
namespace conversion {

bool hasTokens(const std::string&);
std::unique_ptr<expression::Expression> convertTokenStringToFormatExpression(const std::string&);
std::unique_ptr<expression::Expression> convertTokenStringToImageExpression(const std::string&);
std::unique_ptr<expression::Expression> convertTokenStringToExpression(const std::string&);

std::optional<std::unique_ptr<expression::Expression>> convertFunctionToExpression(expression::type::Type,
                                                                                   const Convertible&,
                                                                                   Error&,
                                                                                   bool convertTokens);

template <class T>
std::optional<PropertyExpression<T>> convertFunctionToExpression(const Convertible& value,
                                                                 Error& error,
                                                                 bool convertTokens);

} // namespace conversion
} // namespace style
} // namespace mln
