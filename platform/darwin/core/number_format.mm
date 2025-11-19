#import <Foundation/Foundation.h>

#include <mbgl/i18n/number_format.hpp>

namespace mbgl {
namespace platform {

std::string formatNumber(double number, const std::string& localeId, const std::string& currency,
                         uint8_t minFractionDigits, uint8_t maxFractionDigits) {

    // Create a new formatter each time to avoid state pollution between calls
    NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];

    numberFormatter.locale = !localeId.empty() ? [NSLocale localeWithLocaleIdentifier:@(localeId.c_str())] : nil;
    numberFormatter.currencyCode = !currency.empty() ? @(currency.c_str()) : nil;

    if (currency.empty()) {
        numberFormatter.minimumFractionDigits = minFractionDigits;
        numberFormatter.maximumFractionDigits = maxFractionDigits;
        numberFormatter.numberStyle = NSNumberFormatterDecimalStyle;
    } else {
        numberFormatter.numberStyle = NSNumberFormatterCurrencyStyle;
    }
    NSString *formatted = [numberFormatter stringFromNumber:@(number)];
    std::string result = std::string([formatted UTF8String]);
    return result;
}

} // namespace platform
} // namespace mbgl
