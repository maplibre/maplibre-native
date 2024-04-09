#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/type.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/conversion.hpp>

#include <memory>
#include <vector>
#include <optional>

namespace mbgl {
namespace style {
namespace expression {

namespace detail {
struct SignatureBase;
} // namespace detail

/**
    CompoundExpression provides a mechanism for implementing an expression
    simply by providing a list of pure functions of the form
    (const T0& arg0, const T1& arg1, ...) -> Result<U> where T0, T1, ..., U are
    member types of mbgl::style::expression::Value.

    The majority of expressions specified in the style-spec are implemented in
    this fashion (see compound_expression.cpp).
*/
class CompoundExpression : public Expression {
public:
    CompoundExpression(const detail::SignatureBase&, std::vector<std::unique_ptr<Expression>>);

    std::string getOperator() const override;
    EvaluationResult evaluate(const EvaluationContext& evaluationParams) const override;
    std::vector<std::optional<Value>> possibleOutputs() const override { return {std::nullopt}; }
    void eachChild(const std::function<void(const Expression&)>& visit) const override;
    bool operator==(const Expression& e) const noexcept override;

    std::optional<std::size_t> getParameterCount() const noexcept;

    static bool exists(const std::string& name) noexcept;

protected:
    const detail::SignatureBase& signature;
    std::vector<std::unique_ptr<Expression>> args;
};

ParseResult parseCompoundExpression(const std::string& name,
                                    const mbgl::style::conversion::Convertible& value,
                                    ParsingContext& ctx);

ParseResult createCompoundExpression(const std::string& name,
                                     std::vector<std::unique_ptr<Expression>> args,
                                     ParsingContext& ctx);

} // namespace expression
} // namespace style
} // namespace mbgl
