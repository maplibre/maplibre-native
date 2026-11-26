#pragma once

#include <string_view>

namespace mln {
namespace style {
namespace expression {

size_t unicodeLengthOnValidatedUtf8(std::string_view str);
size_t getUnicodeCharacterOffsetOnValidatedUtf8(std::string_view str, size_t char_offset);

} // namespace expression
} // namespace style
} // namespace mln
