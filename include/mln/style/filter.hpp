#pragma once

#include <mln/util/variant.hpp>
#include <mln/util/feature.hpp>
#include <mln/util/geometry.hpp>
#include <mln/style/expression/expression.hpp>

#include <string>
#include <vector>
#include <tuple>
#include <optional>

namespace mln {
namespace style {

class Filter {
public:
    std::optional<std::shared_ptr<const expression::Expression>> expression;

private:
    static std::size_t hashExpression(const expression::Expression&);

    std::optional<mln::Value> legacyFilter;
    std::size_t contentHash = 0;

public:
    Filter() = default;

    Filter(expression::ParseResult _expression, std::optional<mln::Value> _filter = std::nullopt)
        : expression(std::move(*_expression)),
          legacyFilter(std::move(_filter)),
          contentHash(expression ? hashExpression(**expression) : 0) {
        assert(!expression || *expression != nullptr);
    }

    /// Hash of the expression's contents, computed once on construction. Equal filters have equal
    /// hashes, so it can key a hash map and rule out inequality before a deep comparison.
    std::size_t hash() const noexcept { return contentHash; }

    bool operator()(const expression::EvaluationContext& context) const;

    operator bool() const { return expression || legacyFilter; }

    friend bool operator==(const Filter& lhs, const Filter& rhs) {
        if (!lhs.expression || !rhs.expression) {
            return lhs.expression == rhs.expression;
        }
        // Compare the expressions themselves: every layer parses its own filter, so two layers
        // with the same filter hold different expression objects.
        return *lhs.expression == *rhs.expression ||
               (lhs.contentHash == rhs.contentHash && **lhs.expression == **rhs.expression);
    }

    friend bool operator!=(const Filter& lhs, const Filter& rhs) { return !(lhs == rhs); }

    mln::Value serialize() const {
        if (legacyFilter) {
            return *legacyFilter;
        } else if (expression) {
            return (**expression).serialize();
        }
        return NullValue();
    }
};

} // namespace style
} // namespace mln
