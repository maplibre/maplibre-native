#pragma once

#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/conversion.hpp>

#include <memory>

namespace mln {
namespace style {
namespace expression {

class Literal : public Expression {
public:
    Literal(const Value& value_)
        : Expression(Kind::Literal, typeOf(value_), Dependency::None),
          value(value_) {}

    Literal(Value&& value_)
        : Expression(Kind::Literal, typeOf(value_), Dependency::None),
          value(std::move(value_)) {}

    Literal(type::Array type_, std::vector<Value> value_)
        : Expression(Kind::Literal, std::move(type_), Dependency::None),
          value(std::move(value_)) {}

    EvaluationResult evaluate(const EvaluationContext&) const override { return value; }

    static ParseResult parse(const mln::style::conversion::Convertible&, ParsingContext&);

    void eachChild(const std::function<void(const Expression&)>&) const noexcept override {}

    bool operator==(const Expression& e) const noexcept override {
        if (e.getKind() == Kind::Literal) {
            const auto* rhs = static_cast<const Literal*>(&e);
            return value == rhs->value;
        }
        return false;
    }

    std::vector<std::optional<Value>> possibleOutputs() const override { return {{value}}; }

    const Value& getValue() const noexcept { return value; }

    mln::Value serialize() const override;
    std::string getOperator() const override { return "literal"; }

private:
    Value value;
};

} // namespace expression
} // namespace style
} // namespace mln
