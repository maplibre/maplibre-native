#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/conversion.hpp>

#include <memory>
#include <vector>

namespace mln {
namespace style {
namespace expression {

class Coercion : public Expression {
public:
    Coercion(const type::Type& type_, std::vector<std::unique_ptr<Expression>> inputs_);

    static ParseResult parse(const mln::style::conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& params) const override;
    void eachChild(const std::function<void(const Expression&)>& visit) const override;

    mln::Value serialize() const override;

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override;

    std::string getOperator() const override;

private:
    EvaluationResult (*coerceSingleValue)(const Value& v);
    std::vector<std::unique_ptr<Expression>> inputs;
};

} // namespace expression
} // namespace style
} // namespace mln
