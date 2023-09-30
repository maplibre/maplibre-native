#include <mbgl/util/utf.hpp>

#include <locale>
#include <codecvt>

namespace mbgl {
namespace util {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

std::u16string convertUTF8ToUTF16(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;

    return conv.from_bytes(str);
}

std::string convertUTF16ToUTF8(const std::u16string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;

    return conv.to_bytes(str);
}

#pragma GCC diagnostic pop

} // namespace util
} // namespace mbgl
