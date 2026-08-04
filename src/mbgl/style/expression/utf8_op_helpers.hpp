#pragma once

#include <string_view>

namespace mbgl {
namespace style {
namespace expression {

size_t unicodeLengthOnValidatedUtf8(std::string_view str);
size_t getUnicodeCharacterOffsetOnValidatedUtf8(std::string_view str, size_t char_offset);

} // namespace expression
} // namespace style
} // namespace mbgl
