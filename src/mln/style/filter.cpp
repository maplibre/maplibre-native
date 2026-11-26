#include <mln/style/filter.hpp>
#include <mln/tile/geometry_tile_data.hpp>

namespace mln {
namespace style {

bool Filter::operator()(const expression::EvaluationContext &context) const {
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
