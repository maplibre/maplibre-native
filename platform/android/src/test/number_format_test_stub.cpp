#include <mln/i18n/number_format.hpp>

namespace mln {
namespace platform {

std::string formatNumber(double /*number*/,
                         const std::string& /*localeId */,
                         const std::string& /*currency*/,
                         std::optional<uint8_t> /*minFractionDigits*/,
                         std::optional<uint8_t> /*maxFractionDigits*/
) {
    return "";
}

} // namespace platform
} // namespace mln
