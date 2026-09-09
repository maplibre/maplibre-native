#include <mln/style/expression/semiliteral.hpp>
#include <mln/style/expression/literal.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/util/string.hpp>

namespace mln::style::expression {
namespace {
type::Array arrayType(const std::vector<std::unique_ptr<Expression>>& elements) {
    type::Type itemType = elements.empty() ? type::Value : elements.front()->getType();
    for (const auto& element : elements) {
        if (element->getType() != itemType) {
            itemType = type::Value;
            break;
        }
    }
    return type::Array(itemType, elements.size());
}
} // namespace

Semiliteral::Semiliteral(std::vector<std::unique_ptr<Expression>> elements_)
    : Expression(Kind::Semiliteral, arrayType(elements_), collectDependencies(elements_)),
      elements(std::move(elements_)) {}

ParseResult Semiliteral::parse(const conversion::Convertible& value, ParsingContext& ctx) {
    using namespace conversion;
    if (arrayLength(value) != 2) {
        ctx.error("'semiliteral' expression requires exactly one argument, but found " +
                  util::toString(arrayLength(value) - 1) + " instead.");
        return {};
    }
    const auto array = arrayMember(value, 1);
    if (!isArray(array)) {
        return Literal::parse(value, ctx);
    }
    std::vector<std::unique_ptr<Expression>> elements;
    const auto length = arrayLength(array);
    elements.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        auto element = ctx.parse(arrayMember(array, i), i, type::Value);
        if (!element) return {};
        elements.push_back(std::move(*element));
    }
    return ParseResult(std::make_unique<Semiliteral>(std::move(elements)));
}

EvaluationResult Semiliteral::evaluate(const EvaluationContext& ctx) const {
    std::vector<Value> values;
    values.reserve(elements.size());
    for (const auto& element : elements) {
        auto value = element->evaluate(ctx);
        if (!value) return value;
        values.push_back(std::move(*value));
    }
    return values;
}

void Semiliteral::eachChild(const std::function<void(const Expression&)>& visit) const {
    for (const auto& element : elements) visit(*element);
}

bool Semiliteral::operator==(const Expression& other) const noexcept {
    return other.getKind() == Kind::Semiliteral &&
           childrenEqual(elements, static_cast<const Semiliteral&>(other).elements);
}

mln::Value Semiliteral::serialize() const {
    std::vector<mln::Value> values;
    values.reserve(elements.size());
    for (const auto& element : elements) values.push_back(element->serialize());
    return std::vector<mln::Value>{getOperator(), std::move(values)};
}

} // namespace mln::style::expression
