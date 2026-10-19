#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/conversion.hpp>
#include <mln/style/expression/parsing_context.hpp>

#include <memory>
#include <vector>

namespace mln {
namespace style {
namespace expression {

class Length : public Expression {
public:
    Length(std::unique_ptr<Expression> input);

    static ParseResult parse(const mln::style::conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& params) const override;
    void eachChild(const std::function<void(const Expression&)>& visit) const override;
    bool operator==(const Expression& e) const noexcept override;
    std::vector<std::optional<Value>> possibleOutputs() const override;
    std::string getOperator() const override { return "length"; }

private:
    std::unique_ptr<Expression> input;
};

} // namespace expression
} // namespace style
} // namespace mln
