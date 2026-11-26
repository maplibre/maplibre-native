#pragma once

#include <mln/style/expression/type.hpp>
#include <memory>

namespace mln {
namespace style {
namespace expression {
namespace type {

std::optional<std::string> checkSubtype(const Type& expected, const Type& t);

} // namespace type
} // namespace expression
} // namespace style
} // namespace mln
