#include <mln/style/filter.hpp>
#include <mln/style/expression/literal.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/util/hash.hpp>

namespace mln {
namespace style {

namespace {

// Consistent with expression::Value equality: values that compare equal hash equal. Kinds that
// are not expected in filters (colors, images, ...) hash by kind only.
std::size_t hashValue(const expression::Value& value) {
    using util::hash_combine;
    auto seed = static_cast<std::size_t>(value.which());
    value.match([&](const bool b) { hash_combine(seed, b); },
                // 0.0 and -0.0 compare equal.
                [&](const double d) { hash_combine(seed, d == 0.0 ? 0.0 : d); },
                [&](const std::string& s) { hash_combine(seed, s); },
                [&](const std::vector<expression::Value>& items) {
                    for (const auto& item : items) hash_combine(seed, hashValue(item));
                },
                [&](const std::unordered_map<std::string, expression::Value>& members) {
                    // Member order is unspecified, so combine the members order-independently.
                    std::size_t sum = 0;
                    for (const auto& [key, member] : members) sum += util::hash(key, hashValue(member));
                    hash_combine(seed, sum);
                },
                [](const auto&) {});
    return seed;
}

} // namespace

std::size_t Filter::hashExpression(const expression::Expression& expr) {
    using util::hash_combine;
    std::size_t seed = static_cast<std::size_t>(expr.getKind());
    if (expr.getKind() == expression::Kind::Literal) {
        hash_combine(seed, hashValue(static_cast<const expression::Literal&>(expr).getValue()));
    } else {
        hash_combine(seed, expr.getOperator());
    }
    expr.eachChild([&](const expression::Expression& child) { hash_combine(seed, hashExpression(child)); });
    return seed;
}

bool Filter::operator()(const expression::EvaluationContext& context) const {
    if (!this->expression) return true;

    const expression::EvaluationResult result = (*this->expression)->evaluate(context);
    if (result) {
        const std::optional<bool> typed = expression::fromExpressionValue<bool>(*result);
        return typed ? *typed : false;
    } else {
        return false;
    }
}

} // namespace style
} // namespace mln
