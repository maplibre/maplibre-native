#pragma once

#include <mbgl/util/variant.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/style/expression/expression.hpp>

#include <optional>

namespace mbgl {
namespace style {

class Filter {
public:
    std::optional<std::shared_ptr<const expression::Expression>> expression;

private:
    std::optional<mbgl::Value> legacyFilter;

public:
    Filter() = default;

    Filter(expression::ParseResult _expression, std::optional<mbgl::Value> _filter = std::nullopt)
        : expression(std::move(*_expression)),
          legacyFilter(std::move(_filter)) {
        assert(!expression || *expression != nullptr);
    }

    bool operator()(const expression::EvaluationContext& context) const;

    operator bool() const { return expression || legacyFilter; }

    friend bool operator==(const Filter& lhs, const Filter& rhs) {
        if (lhs.legacyFilter) {
            return lhs.legacyFilter == rhs.legacyFilter;
        }

        if (!lhs.expression || !rhs.expression) {
            return lhs.expression == rhs.expression;
        } else if (*lhs.expression == nullptr || *rhs.expression == nullptr) {
            // checks if both are nullptrs
            return *lhs.expression == *rhs.expression;
        } else {
            return *lhs.expression.value() == *rhs.expression.value();
        }
    }

    friend bool operator!=(const Filter& lhs, const Filter& rhs) { return !(lhs == rhs); }

    mbgl::Value serialize() const {
        if (legacyFilter) {
            return *legacyFilter;
        } else if (expression) {
            return (**expression).serialize();
        }
        return NullValue();
    }
};

} // namespace style
} // namespace mbgl
