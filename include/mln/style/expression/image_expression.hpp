#pragma once

#include <mln/style/expression/expression.hpp>

namespace mln {
namespace style {
namespace expression {

class ParsingContext;

class ImageExpression final : public Expression {
public:
    explicit ImageExpression(std::unique_ptr<Expression> imageID);

    EvaluationResult evaluate(const EvaluationContext&) const override;
    static ParseResult parse(const mln::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override { return {std::nullopt}; }

    mln::Value serialize() const override;
    std::string getOperator() const override { return "image"; }

private:
    std::shared_ptr<Expression> imageID;
};

} // namespace expression
} // namespace style
} // namespace mln
