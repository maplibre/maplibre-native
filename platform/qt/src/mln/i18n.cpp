#include <mln/util/i18n.hpp>

#include <QChar>

namespace mln {
namespace util {
namespace i18n {

bool isDigit(char16_t chr) {
    return QChar(chr).isDigit();
}

bool isUppercase(char16_t chr) {
    return QChar(chr).isUpper();
}

bool isPunctuationOrSymbol(char16_t chr) {
    return QChar(chr).isPunct() || QChar(chr).isSymbol();
}

} // namespace i18n
} // namespace util
} // namespace mln
