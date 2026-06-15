#include <mbgl/style/expression/utf8_op_helpers.hpp>

#include <cstdint>

namespace mbgl {
namespace style {
namespace expression {

namespace {
bool isContinuationByte(uint8_t in) {
    return (in & 0xC0) == 0x80;
}
} // namespace

// get the number of unicode characters of an utf-8 string
// precondition: the string must be valid utf-8 (otherwise, the result may be incorrect)
size_t unicodeLengthOnValidatedUtf8(std::string_view str) {
    size_t length = 0;
    for (size_t i = 0; i < str.size(); i++) {
        uint8_t byte = str[i];
        if (!isContinuationByte(byte)) {
            length++;
        }
    }
    return length;
}
// get the byte offset of a unicode character offset in a utf-8 string
// precondition: the string must be valid utf-8 (otherwise, the result may be incorrect)
// parameters:
// - str: input string
// - char_offset: offset in the string with respect to the unicode character count
// return value:
// - byte offset of the (start of the) unicode character with the requested offset in the string
// - if char_offset is larger than the number of unicode characters or otherwise invalid, the return value is str.size()
size_t getUnicodeCharacterOffsetOnValidatedUtf8(std::string_view str, size_t char_offset) {
    size_t cur_char = 0;
    for (size_t i = 0; i < str.size(); i++) {
        uint8_t byte = str[i];
        if (!isContinuationByte(byte)) {
            if (cur_char == char_offset) {
                return i;
            }
            cur_char++;
        }
    }
    return str.size();
}

} // namespace expression
} // namespace style
} // namespace mbgl
