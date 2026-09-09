#pragma once

#include <mln/style/expression/expression.hpp>

namespace mln::style::expression {

class Semiliteral final : public Expression {
public:
    explicit Semiliteral(std::vector<std::unique_ptr<Expression>>);

    static ParseResult parse(const conversion::Convertible&, ParsingContext&);
    EvaluationResult evaluate(const EvaluationContext&) const override;
    void eachChild(const std::function<void(const Expression&)>&) const override;
    bool operator==(const Expression&) const noexcept override;
    std::vector<std::optional<Value>> possibleOutputs() const override { return {std::nullopt}; }
    mln::Value serialize() const override;
    std::string getOperator() const override { return "semiliteral"; }

private:
    std::vector<std::unique_ptr<Expression>> elements;
};

} // namespace mln::style::expression
