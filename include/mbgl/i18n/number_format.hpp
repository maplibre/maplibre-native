#pragma once

#include <cstdint>
#include <string>

namespace mln {
namespace platform {

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         uint8_t minFractionDigits,
                         uint8_t maxFractionDigits);

} // namespace platform
} // namespace mln
