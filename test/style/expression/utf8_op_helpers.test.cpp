#include <mbgl/test/util.hpp>

#include <mbgl/style/expression/utf8_op_helpers.hpp>

using namespace mbgl::style::expression;

TEST(Utf8OpHelpers, unicodeLengthOnValidatedUtf8) {
    EXPECT_EQ(unicodeLengthOnValidatedUtf8(""), 0);
    EXPECT_EQ(unicodeLengthOnValidatedUtf8("Hello, world!"), 13);
    EXPECT_EQ(unicodeLengthOnValidatedUtf8("aäü"), 3);
    EXPECT_EQ(unicodeLengthOnValidatedUtf8("オオオ"), 3);
    EXPECT_EQ(unicodeLengthOnValidatedUtf8("𝄆𝄆"), 2);
    EXPECT_EQ(unicodeLengthOnValidatedUtf8("aöオ𝄆オöa"), 7);
}
TEST(Utf8OpHelpers, getUnicodeCharacterOffsetOnValidatedUtf8) {
    // edge case: empty string
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8("", 0), 0);

    std::string str = "aöオ𝄆オöa";
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 0), 0);  // 'a'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 1), 1);  // 'ö'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 2), 3);  // 'オ'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 3), 6);  // '𝄆'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 4), 10); // 'オ'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 5), 13); // 'ö'
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 6), 15); // 'a'

    // Test out of bounds character offset (should return the string length)
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 7), 16);
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 12), 16);
    EXPECT_EQ(getUnicodeCharacterOffsetOnValidatedUtf8(str, 50000), 16);
}
