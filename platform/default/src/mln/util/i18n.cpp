#include <mln/util/i18n.hpp>

#include <unicode/uchar.h>

namespace mln {
namespace util {
namespace i18n {

bool isDigit(char16_t chr) {
    return u_charType(chr) == U_DECIMAL_DIGIT_NUMBER;
}

bool isUppercase(char16_t chr) {
    return u_charType(chr) == U_UPPERCASE_LETTER;
}

bool isPunctuationOrSymbol(char16_t chr) {
    return (U_GET_GC_MASK(chr) & (U_GC_P_MASK | U_GC_S_MASK)) != 0;
}

} // namespace i18n
} // namespace util
} // namespace mln
