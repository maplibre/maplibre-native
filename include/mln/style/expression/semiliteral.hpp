#pragma once

#include <mln/style/expression/expression.hpp>

namespace mln::style::expression {

class Semiliteral final : public Expression {
public:
    explicit Semiliteral(std::vector<std::unique_ptr<Expression>> elements_);

    static ParseResult parse(const conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& ctx) const override;
    void eachChild(const std::function<void(const Expression&)>& visit) const override;

    bool operator==(const Expression& other) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override { return {std::nullopt}; }

    mln::Value serialize() const override;
    std::string getOperator() const override { return "semiliteral"; }

private:
    std::vector<std::unique_ptr<Expression>> elements;
};

} // namespace mln::style::expression
