#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/conversion.hpp>

#include <memory>
#include <map>

namespace mln {
namespace style {
namespace expression {

class Coalesce : public Expression {
public:
    using Args = std::vector<std::unique_ptr<Expression>>;
    Coalesce(type::Type type_, Args args_)
        : Expression(Kind::Coalesce, std::move(type_), collectDependencies(args_)),
          args(std::move(args_)) {}

    static ParseResult parse(const mln::style::conversion::Convertible& value, ParsingContext& ctx);

    EvaluationResult evaluate(const EvaluationContext& params) const override;

    void eachChild(const std::function<void(const Expression&)>& visit) const override;

    bool operator==(const Expression& e) const noexcept override;

    std::vector<std::optional<Value>> possibleOutputs() const override;

    std::size_t getLength() const noexcept { return args.size(); }

    Expression* getChild(std::size_t i) const { return args.at(i).get(); }

    std::string getOperator() const override { return "coalesce"; }

private:
    Args args;
};

} // namespace expression
} // namespace style
} // namespace mln
