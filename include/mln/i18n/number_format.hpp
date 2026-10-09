#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace mln {
namespace platform {

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         std::optional<uint8_t> minFractionDigits,
                         std::optional<uint8_t> maxFractionDigits);

} // namespace platform
} // namespace mln
