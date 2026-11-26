#pragma once

#include <cstdint>
#include <string>

namespace mln {
namespace platform {

// Implementations report recoverable failures, such as text they cannot
// convert, by throwing util::LocaleException. Style expressions evaluate such a
// failure as an expression error. Other exceptions propagate.
std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits);

} // namespace platform
} // namespace mln
