#pragma once

#include <mln/style/expression/collator_expression.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/conversion.hpp>

#include <memory>

namespace mln {
namespace style {
namespace expression {

ParseResult parseComparison(const mln::style::conversion::Convertible&, ParsingContext&);

class BasicComparison : public Expression {
public:
    using CompareFunctionType = bool (*)(const Value&, const Value&);

    BasicComparison(std::string op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);

    void eachChild(const std::function<void(const Expression&)>& visit) const override;
    bool operator==(const Expression&) const noexcept override;
    EvaluationResult evaluate(const EvaluationContext&) const override;
    std::vector<std::optional<Value>> possibleOutputs() const override;
    std::string getOperator() const override { return op; }

private:
    std::string op;
    CompareFunctionType compare;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
    bool needsRuntimeTypeCheck;
};

class CollatorComparison : public Expression {
public:
    using CompareFunctionType = bool (*)(const std::string&, const std::string&, const Collator&);

    CollatorComparison(std::string op,
                       std::unique_ptr<Expression> lhs,
                       std::unique_ptr<Expression> rhs,
                       std::unique_ptr<Expression> collator);

    void eachChild(const std::function<void(const Expression&)>& visit) const override;
    bool operator==(const Expression&) const noexcept override;
    EvaluationResult evaluate(const EvaluationContext&) const override;
    std::vector<std::optional<Value>> possibleOutputs() const override;
    std::string getOperator() const override { return op; }

private:
    std::string op;
    CompareFunctionType compare;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;
    std::unique_ptr<Expression> collator;
    bool needsRuntimeTypeCheck;
};

} // namespace expression
} // namespace style
} // namespace mln
