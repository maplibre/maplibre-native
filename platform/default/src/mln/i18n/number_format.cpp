#include <mln/i18n/number_format.hpp>

#if !defined(MBGL_USE_BUILTIN_ICU)
#include <unicode/numberformatter.h>
#include <unicode/ucurr.h>
#include <unicode/uvernum.h>
#endif

#include <algorithm>
#include <string>

namespace mln {
namespace platform {

#if !defined(MBGL_USE_BUILTIN_ICU)
std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         std::optional<uint8_t> minFractionDigits,
                         std::optional<uint8_t> maxFractionDigits) {
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString ustr;
    std::string formatted;

    icu::Locale locale = icu::Locale(localeId.c_str());
    // Print the value as currency
    if (!currency.empty()) {
        icu::UnicodeString ucurrency = icu::UnicodeString::fromUTF8(currency);
        auto formatter =
            icu::number::NumberFormatter::with().unit(icu::CurrencyUnit(ucurrency.getBuffer(), status)).locale(locale);
        if (minFractionDigits || maxFractionDigits) {
            // Fill in an omitted limit from the currency's own digits, as GL JS does.
            const int currencyDigits = ucurr_getDefaultFractionDigits(ucurrency.getTerminatedBuffer(), &status);
            const int minDigits = minFractionDigits.value_or(
                std::min<int>(currencyDigits, maxFractionDigits.value_or(currencyDigits)));
            const int maxDigits = maxFractionDigits.value_or(std::max(minDigits, currencyDigits));
            formatter = formatter.precision(icu::number::Precision::minMaxFraction(minDigits, maxDigits));
        }
        ustr = formatter.formatDouble(number, status).toString(status);
    } else {
        ustr = icu::number::NumberFormatter::with()
                   .precision(icu::number::Precision::minMaxFraction(minFractionDigits.value_or(0),
                                                                     maxFractionDigits.value_or(3)))
                   .locale(locale)
                   .formatDouble(number, status)
                   .toString(status);
    }
    return ustr.toUTF8String(formatted);
}
#else
std::string formatNumber(
    double number, const std::string&, const std::string&, std::optional<uint8_t>, std::optional<uint8_t>) {
    return std::to_string(number);
}
#endif

} // namespace platform
} // namespace mln
