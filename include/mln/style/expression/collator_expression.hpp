#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/conversion.hpp>

#include <memory>

namespace mln {
namespace style {
namespace expression {

class CollatorExpression : public Expression {
public:
    CollatorExpression(std::unique_ptr<Expression> caseSensitive,
                       std::unique_ptr<Expression> diacriticSensitive,
                       std::optional<std::unique_ptr<Expression>> locale);

    EvaluationResult evaluate(const EvaluationContext&) const override;
    static ParseResult parse(const mln::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression&) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override {
        // Technically the set of possible outputs is the combinatoric set of
        // Collators produced by all possibleOutputs of
        // locale/caseSensitive/diacriticSensitive But for the primary use of
        // Collators in comparison operators, we ignore the Collator's possibleOutputs
        // anyway, so we can get away with leaving this undefined for now.
        return {std::nullopt};
    }

    mln::Value serialize() const override;
    std::string getOperator() const override { return "collator"; }

private:
    std::unique_ptr<Expression> caseSensitive;
    std::unique_ptr<Expression> diacriticSensitive;
    std::optional<std::unique_ptr<Expression>> locale;
};

} // namespace expression
} // namespace style
} // namespace mln
