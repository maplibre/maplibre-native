#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/conversion.hpp>
#include <memory>

namespace mbgl {
namespace style {
namespace expression {

class IndexOf : public Expression {
public:
    IndexOf(std::unique_ptr<Expression> keyword_,
            std::unique_ptr<Expression> input_,
            std::unique_ptr<Expression> fromIndex_);

    static ParseResult parse(const mbgl::style::conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& params) const override;
    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression& e) const override;

    std::vector<std::optional<Value>> possibleOutputs() const override;
    std::string getOperator() const override;

private:
    EvaluationResult evaluateForArrayInput(const std::vector<Value>& array,
                                           const Value& keyword,
                                           size_t fromIndex) const;
    EvaluationResult evaluateForStringInput(const std::string& string, const Value& keyword, size_t fromIndex) const;

private:
    std::unique_ptr<Expression> keyword;
    std::unique_ptr<Expression> input;
    std::unique_ptr<Expression> fromIndex;
};

} // namespace expression
} // namespace style
} // namespace mbgl
