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
    std::optional<mln::Value> legacyFilter;

public:
    Filter() = default;

    Filter(expression::ParseResult _expression, std::optional<mln::Value> _filter = std::nullopt)
        : expression(std::move(*_expression)),
          legacyFilter(std::move(_filter)) {
        assert(!expression || *expression != nullptr);
    }

    bool operator()(const expression::EvaluationContext& context) const;

    operator bool() const { return expression || legacyFilter; }

    friend bool operator==(const Filter& lhs, const Filter& rhs) {
        if (!lhs.expression || !rhs.expression) {
            return lhs.expression == rhs.expression;
        } else {
            return *(lhs.expression) == *(rhs.expression);
        }
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
