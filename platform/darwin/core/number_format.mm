#import <Foundation/Foundation.h>

#include <mln/i18n/number_format.hpp>

namespace mln {
namespace platform {

std::string formatNumber(double number, const std::string& localeId, const std::string& currency,
                         std::optional<uint8_t> minFractionDigits,
                         std::optional<uint8_t> maxFractionDigits) {
  // Create a new formatter each time to avoid state pollution between calls
  NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];

  numberFormatter.locale =
      !localeId.empty() ? [NSLocale localeWithLocaleIdentifier:@(localeId.c_str())] : nil;
  numberFormatter.currencyCode = !currency.empty() ? @(currency.c_str()) : nil;

  if (currency.empty()) {
    numberFormatter.minimumFractionDigits = minFractionDigits.value_or(0);
    numberFormatter.maximumFractionDigits = maxFractionDigits.value_or(3);
    numberFormatter.numberStyle = NSNumberFormatterDecimalStyle;
  } else {
    numberFormatter.numberStyle = NSNumberFormatterCurrencyStyle;
  }
  NSString* formatted = [numberFormatter stringFromNumber:@(number)];
  std::string result = std::string([formatted UTF8String]);
  return result;
}

}  // namespace platform
}  // namespace mln
