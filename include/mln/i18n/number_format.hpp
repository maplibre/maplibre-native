#pragma once

#include <cstdint>
#include <string>

namespace mln {
namespace platform {

// Implementations report failures, such as an unsupported locale or currency,
// by throwing an exception derived from std::exception. Style expressions
// evaluate such a failure as an expression error. std::bad_alloc is not treated
// as an expression error.
std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits);

} // namespace platform
} // namespace mln
