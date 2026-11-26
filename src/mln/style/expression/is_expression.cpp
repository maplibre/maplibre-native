#include <mln/style/expression/is_expression.hpp>
#include <mln/style/expression/compound_expression.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/conversion_impl.hpp>

#include <unordered_set>

namespace mln {
namespace style {
namespace expression {

using namespace mln::style::conversion;

bool isExpression(const Convertible& value) {
    if (!isArray(value) || arrayLength(value) == 0) return false;
    std::optional<std::string> name = toString(arrayMember(value, 0));
    if (!name) return false;

    return isExpression(*name) || CompoundExpression::exists(*name);
}

} // namespace expression
} // namespace style
} // namespace mln
