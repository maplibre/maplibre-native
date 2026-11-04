#include <mln/i18n/number_format.hpp>

#include <QLocale>
#include <QString>

namespace mln {
namespace platform {

std::string formatNumber(double number,
                         const std::string& localeId,
                         const std::string& currency,
                         std::optional<uint8_t> minFractionDigits,
                         std::optional<uint8_t> maxFractionDigits) {
    QString formatted;
    // Qt Locale::toString() API takes only one precision argument
    (void)minFractionDigits;
    QLocale locale = QLocale(QString::fromStdString(localeId));

    if (!currency.empty()) {
        formatted = locale.toCurrencyString(number);
    } else {
        formatted = locale.toString(number, 'f', maxFractionDigits.value_or(3));
    }
    return formatted.toStdString();
}

} // namespace platform
} // namespace mln
