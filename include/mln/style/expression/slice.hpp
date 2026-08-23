#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/conversion.hpp>
#include <memory>

namespace mln {
namespace style {
namespace expression {

class Slice : public Expression {
public:
    Slice(std::unique_ptr<Expression> input_,
          std::unique_ptr<Expression> fromIndex_,
          std::unique_ptr<Expression> toIndex_);

    static ParseResult parse(const mln::style::conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& params) const override;
    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override;
    std::string getOperator() const override { return "slice"; }

private:
    EvaluationResult evaluateForStringInput(const std::string& input, int fromIndexValue, int toIndexValue) const;
    EvaluationResult evaluateForArrayInput(const std::vector<Value>& input, int fromIndexValue, int toIndexValue) const;

private:
    std::unique_ptr<Expression> input;
    std::unique_ptr<Expression> fromIndex;
    std::unique_ptr<Expression> toIndex;
};

} // namespace expression
} // namespace style
} // namespace mln
