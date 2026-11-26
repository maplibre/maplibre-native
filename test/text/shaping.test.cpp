
#include <mln/test/util.hpp>

#include <mln/text/bidi.hpp>
#include <mln/text/tagged_string.hpp>
#include <mln/text/shaping.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/i18n.hpp>

#include <optional>

using namespace mln;
using namespace util;

namespace {

void addStubGlyph(GlyphMap& glyphs,
                  GlyphPositions& glyphPositions,
                  const FontStackHash fontStackHash,
                  const char16_t codePoint,
                  const GlyphIDType type) {
    const GlyphID glyphID(codePoint, type);
    GlyphMetrics metrics;
    metrics.width = 10;
    metrics.height = 18;
    metrics.left = 1;
    metrics.top = -8;
    metrics.advance = 12;

    Glyph glyph;
    glyph.id = glyphID;
    glyph.metrics = metrics;
    glyphs[fontStackHash][glyphID] = Immutable<Glyph>(makeMutable<Glyph>(std::move(glyph)));

    GlyphPosition position;
    position.metrics = metrics;
    glyphPositions[fontStackHash][glyphID] = position;
}

void addStubGlyphs(const TaggedString& string, GlyphMap& glyphs, GlyphPositions& glyphPositions) {
    for (std::size_t i = 0; i < string.length(); ++i) {
        const auto codePoint = string.getCharCodeAt(i);

        const auto& section = string.getSection(i);
        if (section.imageID) continue;

        addStubGlyph(glyphs, glyphPositions, section.fontStackHash, codePoint, section.type);
        if (section.type == GlyphIDType::FontPBF) {
            if (const auto verticalCodePoint = util::i18n::verticalizePunctuation(codePoint)) {
                addStubGlyph(glyphs, glyphPositions, section.fontStackHash, verticalCodePoint, section.type);
            }
        }
    }
}

Shaping shapeTaggedText(const TaggedString& string,
                        const WritingModeType writingMode = WritingModeType::Vertical,
                        const bool allowVerticalPlacement = false,
                        ImagePositions imagePositions = {}) {
    GlyphMap glyphs;
    GlyphPositions glyphPositions;
    addStubGlyphs(string, glyphs, glyphPositions);

    BiDi bidi;
    return getShaping(string,
                      4096.0f,
                      ONE_EM,
                      style::SymbolAnchorType::TopLeft,
                      style::TextJustifyType::Left,
                      0.0f,
                      {{0.0f, 0.0f}},
                      writingMode,
                      bidi,
                      glyphs,
                      glyphPositions,
                      imagePositions,
                      ONE_EM,
                      ONE_EM,
                      allowVerticalPlacement);
}

Shaping shapeLineLabel(const std::u16string& text,
                       const WritingModeType writingMode = WritingModeType::Vertical,
                       const bool allowVerticalPlacement = false) {
    const FontStack fontStack{{"font-stack"}};
    return shapeTaggedText(TaggedString(text, SectionOptions(1.0, fontStack, GlyphIDType::FontPBF, 0)),
                           writingMode,
                           allowVerticalPlacement);
}

using GlyphOrientation = std::pair<char16_t, std::string>;

std::vector<GlyphOrientation> getGlyphOrientations(const Shaping& shaping) {
    std::vector<GlyphOrientation> result;
    for (const auto& line : shaping.positionedLines) {
        for (const auto& glyph : line.positionedGlyphs) {
            result.emplace_back(glyph.glyph.complex.code, glyph.vertical ? "upright" : "along-line");
        }
    }
    return result;
}

} // namespace

TEST(Shaping, ZWSP) {
    GlyphPosition glyphPosition;
    glyphPosition.metrics.width = 18;
    glyphPosition.metrics.height = 18;
    glyphPosition.metrics.left = 2;
    glyphPosition.metrics.top = -8;
    glyphPosition.metrics.advance = 21;

    Glyph glyph;
    glyph.id = u'中';
    glyph.metrics = glyphPosition.metrics;

    BiDi bidi;
    auto immutableGlyph = Immutable<Glyph>(makeMutable<Glyph>(std::move(glyph)));
    const std::vector<std::string> fontStack{{"font-stack"}};
    const SectionOptions sectionOptions(1.0f, fontStack, GlyphIDType::FontPBF, 0);
    const float layoutTextSize = 16.0f;
    const float layoutTextSizeAtBucketZoomLevel = 16.0f;
    GlyphMap glyphs = {{FontStackHasher()(fontStack), {{u'中', std::move(immutableGlyph)}}}};
    GlyphPositions glyphPositions = {{FontStackHasher()(fontStack), {{u'中', std::move(glyphPosition)}}}};
    ImagePositions imagePositions;

    const auto testGetShaping = [&](const TaggedString& string, unsigned maxWidthInChars) {
        return getShaping(string,
                          maxWidthInChars * ONE_EM,
                          ONE_EM, // lineHeight
                          style::SymbolAnchorType::Center,
                          style::TextJustifyType::Center,
                          0,              // spacing
                          {{0.0f, 0.0f}}, // translate
                          WritingModeType::Horizontal,
                          bidi,
                          glyphs,
                          glyphPositions,
                          imagePositions,
                          layoutTextSize,
                          layoutTextSizeAtBucketZoomLevel,
                          /*allowVerticalPlacement*/ false);
    };

    // 3 lines
    // 中中中中中中
    // 中中中中中中
    // 中中
    {
        TaggedString string(u"中中\u200b中中\u200b中中\u200b中中中中中中\u200b中中", sectionOptions);
        auto shaping = testGetShaping(string, 5);
        ASSERT_EQ(shaping.positionedLines.size(), 3);
        ASSERT_EQ(shaping.top, -36);
        ASSERT_EQ(shaping.bottom, 36);
        ASSERT_EQ(shaping.left, -63);
        ASSERT_EQ(shaping.right, 63);
        ASSERT_EQ(shaping.writingMode, WritingModeType::Horizontal);
    }

    // 2 lines
    // 中中
    // 中
    {
        TaggedString string(u"中中\u200b中", sectionOptions);
        auto shaping = testGetShaping(string, 1);
        ASSERT_EQ(shaping.positionedLines.size(), 2);
        ASSERT_EQ(shaping.top, -24);
        ASSERT_EQ(shaping.bottom, 24);
        ASSERT_EQ(shaping.left, -21);
        ASSERT_EQ(shaping.right, 21);
        ASSERT_EQ(shaping.writingMode, WritingModeType::Horizontal);
    }

    // 1 line
    // 中中
    {
        TaggedString string(u"中中\u200b", sectionOptions);
        auto shaping = testGetShaping(string, 2);
        ASSERT_EQ(shaping.positionedLines.size(), 1);
        ASSERT_EQ(shaping.top, -12);
        ASSERT_EQ(shaping.bottom, 12);
        ASSERT_EQ(shaping.left, -21);
        ASSERT_EQ(shaping.right, 21);
        ASSERT_EQ(shaping.writingMode, WritingModeType::Horizontal);
    }

    // 5 'new' lines.
    {
        TaggedString string(u"\u200b\u200b\u200b\u200b\u200b", sectionOptions);
        auto shaping = testGetShaping(string, 1);
        ASSERT_EQ(shaping.positionedLines.size(), 5);
        ASSERT_EQ(shaping.top, -60);
        ASSERT_EQ(shaping.bottom, 60);
        ASSERT_EQ(shaping.left, 0);
        ASSERT_EQ(shaping.right, 0);
        ASSERT_EQ(shaping.writingMode, WritingModeType::Horizontal);
    }
}

// Regression cases ported from maplibre-gl-js#8205 for maplibre-native#4565.
TEST(Shaping, VerticalDigitsBetweenCJKCharacters) {
    const auto shapedLineLabel = shapeLineLabel(u"반포대로21길");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'반', "upright"},
        {u'포', "upright"},
        {u'대', "upright"},
        {u'로', "upright"},
        {u'2', "upright"},
        {u'1', "upright"},
        {u'길', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalTrailingDigits) {
    const auto shapedLineLabel = shapeLineLabel(u"身什戰33");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
        {u'3', "upright"},
        {u'3', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalDigitsOfAnyLength) {
    const auto shapedLineLabel = shapeLineLabel(u"国道1234号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'1', "upright"},
        {u'2', "upright"},
        {u'3', "upright"},
        {u'4', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalWhitespaceSeparatedDigit) {
    const auto shapedLineLabel = shapeLineLabel(u"身什戰 1");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
        {u' ', "along-line"},
        {u'1', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalSingleUppercaseLetter) {
    const auto shapedLineLabel = shapeLineLabel(u"国道A号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'A', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalDecomposedLatinLetter) {
    const auto shapedLineLabel = shapeLineLabel(u"国道e\u0301号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'e', "along-line"},
        {u'\u0301', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalLowercaseGreekLetter) {
    const auto shapedLineLabel = shapeLineLabel(u"国道α号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'α', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalNonLatinUppercaseCode) {
    const auto shapedLineLabel = shapeLineLabel(u"国道\u041C1号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'\u041C', "upright"},
        {u'1', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalComplexScript) {
    const auto shapedLineLabel = shapeLineLabel(u"国道ب号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'ب', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalProlongedSoundMark) {
    const auto shapedLineLabel = shapeLineLabel(u"札幌タワー");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'札', "upright"},
        {u'幌', "upright"},
        {u'タ', "upright"},
        {u'ワ', "upright"},
        {u'ー', "along-line"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalWaveDashBetweenDigits) {
    const auto shapedLineLabel = shapeLineLabel(u"身什戰1〜2");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
        {u'1', "upright"},
        {u'〜', "along-line"},
        {u'2', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalLowercaseLatinWord) {
    const auto shapedLineLabel = shapeLineLabel(u"two 身什戰");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u't', "along-line"},
        {u'w', "along-line"},
        {u'o', "along-line"},
        {u' ', "along-line"},
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalMixedAlphanumericCode) {
    const auto shapedLineLabel = shapeLineLabel(u"身什戰A1");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
        {u'A', "upright"},
        {u'1', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalLatinWordAdjoiningCJK) {
    const auto shapedLineLabel = shapeLineLabel(u"銀座Ginza通り");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'銀', "upright"},
        {u'座', "upright"},
        {u'G', "along-line"},
        {u'i', "along-line"},
        {u'n', "along-line"},
        {u'z', "along-line"},
        {u'a', "along-line"},
        {u'通', "upright"},
        {u'り', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalLongUppercaseWord) {
    const auto shapedLineLabel = shapeLineLabel(u"ヴィラ ISHIKAWA");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'ヴ', "upright"},
        {u'ィ', "upright"},
        {u'ラ', "upright"},
        {u' ', "along-line"},
        {u'I', "along-line"},
        {u'S', "along-line"},
        {u'H', "along-line"},
        {u'I', "along-line"},
        {u'K', "along-line"},
        {u'A', "along-line"},
        {u'W', "along-line"},
        {u'A', "along-line"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalShortUppercaseCode) {
    const auto shapedLineLabel = shapeLineLabel(u"JR山手線");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'J', "upright"},
        {u'R', "upright"},
        {u'山', "upright"},
        {u'手', "upright"},
        {u'線', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalPunctuationBetweenDigits) {
    const auto shapedLineLabel = shapeLineLabel(u"国道1-2号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'1', "upright"},
        {u'︲', "upright"},
        {u'2', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalPunctuationInRotatedRun) {
    const auto shapedLineLabel = shapeLineLabel(u"国道3.5km");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'3', "along-line"},
        {u'.', "along-line"},
        {u'5', "along-line"},
        {u'k', "along-line"},
        {u'm', "along-line"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalMissingGlyphPreservesIndices) {
    const FontStack fontStack{{"font-stack"}};
    const auto fontStackHash = FontStackHasher()(fontStack);
    const TaggedString text(u"what 国21号", SectionOptions(1.0, fontStack, GlyphIDType::FontPBF, 0));
    GlyphMap glyphs;
    GlyphPositions glyphPositions;
    addStubGlyphs(text, glyphs, glyphPositions);
    // Missing "w" must not shift the character index used for the remaining glyphs.
    glyphs.at(fontStackHash).erase(u'w');
    glyphPositions.at(fontStackHash).erase(u'w');

    BiDi bidi;
    const auto shapedLineLabel = getShaping(text,
                                            4096.0f,
                                            ONE_EM,
                                            style::SymbolAnchorType::TopLeft,
                                            style::TextJustifyType::Left,
                                            0.0f,
                                            {{0.0f, 0.0f}},
                                            WritingModeType::Vertical,
                                            bidi,
                                            glyphs,
                                            glyphPositions,
                                            {},
                                            ONE_EM,
                                            ONE_EM,
                                            false);
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'h', "along-line"},
        {u'a', "along-line"},
        {u't', "along-line"},
        {u' ', "along-line"},
        {u'国', "upright"},
        {u'2', "upright"},
        {u'1', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, HorizontalGlyphOrientation) {
    const auto shapedLineLabel = shapeLineLabel(u"身什戰33", WritingModeType::Horizontal);
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'身', "along-line"},
        {u'什', "along-line"},
        {u'戰', "along-line"},
        {u'3', "along-line"},
        {u'3', "along-line"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, AllowedVerticalGlyphOrientation) {
    const auto shapedLineLabel = shapeLineLabel(u"two 身什戰", WritingModeType::Vertical, true);
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u't', "upright"},
        {u'w', "upright"},
        {u'o', "upright"},
        {u' ', "along-line"},
        {u'身', "upright"},
        {u'什', "upright"},
        {u'戰', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalThreeCharacterUppercaseCode) {
    const auto shapedLineLabel = shapeLineLabel(u"国道ABC号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'A', "upright"},
        {u'B', "upright"},
        {u'C', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalFourCharacterUppercaseWord) {
    const auto shapedLineLabel = shapeLineLabel(u"国道ABCD号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'A', "along-line"},
        {u'B', "along-line"},
        {u'C', "along-line"},
        {u'D', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalBengaliDigits) {
    const auto shapedLineLabel = shapeLineLabel(u"国道১২号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'১', "upright"},
        {u'২', "upright"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalArabicIndicDigits) {
    const auto shapedLineLabel = shapeLineLabel(u"国道١٢号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'١', "along-line"},
        {u'٢', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalDigitWithCombiningMark) {
    const auto shapedLineLabel = shapeLineLabel(u"国道1\u0301号");
    const auto orientations = getGlyphOrientations(shapedLineLabel);

    const std::vector<GlyphOrientation> expected = {
        {u'国', "upright"},
        {u'道', "upright"},
        {u'1', "along-line"},
        {u'\u0301', "along-line"},
        {u'号', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalGlyphOrientationRespectsFormattedSections) {
    const FontStack fontStack{{"font-stack"}};
    TaggedString text;
    text.addTextSection(u"AB", 1.0, fontStack, GlyphIDType::FontPBF);
    text.addTextSection(u"CD", 0.5, fontStack, GlyphIDType::FontPBF);

    const auto shapedLineLabel = shapeTaggedText(text);
    const auto orientations = getGlyphOrientations(shapedLineLabel);
    const std::vector<GlyphOrientation> expected = {
        {u'A', "along-line"},
        {u'B', "along-line"},
        {u'C', "along-line"},
        {u'D', "along-line"},
    };
    EXPECT_EQ(orientations, expected);
}

TEST(Shaping, VerticalGlyphOrientationDelimitsInlineImages) {
    const FontStack fontStack{{"font-stack"}};
    TaggedString text;
    text.addTextSection(u"AB", 1.0, fontStack, GlyphIDType::FontPBF);
    text.addImageSection("image");
    text.addTextSection(u"CD", 1.0, fontStack, GlyphIDType::FontPBF);

    const style::Image::Impl image("image", PremultipliedImage({12, 24}), 1.0f);
    ImagePositions imagePositions;
    imagePositions.emplace("image", ImagePosition(Rect<uint16_t>(0, 0, 14, 26), image));

    const auto shapedLineLabel = shapeTaggedText(text, WritingModeType::Vertical, false, std::move(imagePositions));
    ASSERT_EQ(shapedLineLabel.positionedLines.size(), 1u);
    ASSERT_EQ(shapedLineLabel.positionedLines.front().positionedGlyphs.size(), 5u);
    const auto orientations = getGlyphOrientations(shapedLineLabel);
    const std::vector<GlyphOrientation> expected = {
        {u'A', "upright"},
        {u'B', "upright"},
        {u'\uE000', "along-line"}, // Inline image placeholder.
        {u'C', "upright"},
        {u'D', "upright"},
    };
    EXPECT_EQ(orientations, expected);
    EXPECT_EQ(shapedLineLabel.positionedLines.front().positionedGlyphs[2].imageID, std::optional<std::string>("image"));
    EXPECT_TRUE(shapedLineLabel.iconsInText);
}

TEST(Shaping, VerticalGlyphOrientationDelimitsNativeGlyphSections) {
    const FontStack fontStack{{"font-stack"}};
    const auto nativeType = genNewGlyphIDType("shaping-test-native", fontStack, {{0, 0xffff}});
    TaggedString text;
    text.addTextSection(u"AB", 1.0, fontStack, GlyphIDType::FontPBF);
    text.addTextSection(u"C", 1.0, fontStack, nativeType);
    text.addTextSection(u"D", 1.0, fontStack, GlyphIDType::FontPBF);

    const auto shapedLineLabel = shapeTaggedText(text);
    const auto orientations = getGlyphOrientations(shapedLineLabel);
    const std::vector<GlyphOrientation> expected = {
        {u'A', "upright"},
        {u'B', "upright"},
        {u'C', "along-line"},
        {u'D', "upright"},
    };
    EXPECT_EQ(orientations, expected);
}

void setupShapedText(Shaping& shapedText, float textSize) {
    const auto glyph = PositionedGlyph(32,
                                       0.0f,
                                       0.0f,
                                       false,
                                       0,
                                       1.0,
                                       /*texRect*/ {},
                                       /*metrics*/ {},
                                       /*imageID*/ std::nullopt);
    shapedText.right = textSize;
    shapedText.bottom = textSize;
    shapedText.positionedLines.emplace_back();
    shapedText.positionedLines.back().positionedGlyphs.emplace_back(glyph);
}

void testApplyTextFit(const Rect<uint16_t>& rectangle,
                      const style::ImageContent& content,
                      const std::optional<style::TextFit> textFitWidth,
                      const std::optional<style::TextFit> textFitHeight,
                      const Shaping& shapedText,
                      float fontScale,
                      float expectedRight,
                      float expectedBottom) {
    ImagePosition image = {
        rectangle,
        style::Image::Impl("test",
                           PremultipliedImage({static_cast<uint32_t>(rectangle.w), static_cast<uint32_t>(rectangle.h)}),
                           1.0f,
                           false,
                           {},
                           {},
                           content,
                           textFitWidth,
                           textFitHeight)};
    auto shapedIcon = PositionedIcon::shapeIcon(image, {0, 0}, style::SymbolAnchorType::TopLeft);
    shapedIcon.fitIconToText(shapedText, style::IconTextFitType::Both, {0, 0, 0, 0}, {0, 0}, fontScale);
    const auto& icon = shapedIcon.applyTextFit();
    ASSERT_EQ(icon.top(), 0);
    ASSERT_EQ(icon.left(), 0);
    ASSERT_EQ(icon.right(), expectedRight);
    ASSERT_EQ(icon.bottom(), expectedBottom);
}

TEST(Shaping, BreakBeforeLeftParenthesis) {
    GlyphPosition glyphPosition;
    glyphPosition.metrics.width = 18;
    glyphPosition.metrics.height = 18;
    glyphPosition.metrics.left = 2;
    glyphPosition.metrics.top = -8;
    glyphPosition.metrics.advance = 21;

    BiDi bidi;
    const std::vector<std::string> fontStack{{"font-stack"}};
    const SectionOptions sectionOptions(1.0f, fontStack, GlyphIDType::FontPBF, 0);
    GlyphMap glyphs;
    GlyphPositions glyphPositions;
    for (const char16_t id : {u'o', u'a', u'(', u')', u' '}) {
        Glyph glyph;
        glyph.id = id;
        glyph.metrics = glyphPosition.metrics;
        glyphs[FontStackHasher()(fontStack)].emplace(id, Immutable<Glyph>(makeMutable<Glyph>(std::move(glyph))));
        glyphPositions[FontStackHasher()(fontStack)].emplace(id, glyphPosition);
    }
    ImagePositions imagePositions;

    const auto glyphsPerLine = [&](const std::u16string& text) {
        const Shaping shaping = getShaping(TaggedString(text, sectionOptions),
                                           5 * ONE_EM,
                                           ONE_EM, // lineHeight
                                           style::SymbolAnchorType::Center,
                                           style::TextJustifyType::Center,
                                           0,              // spacing
                                           {{0.0f, 0.0f}}, // translate
                                           WritingModeType::Horizontal,
                                           bidi,
                                           glyphs,
                                           glyphPositions,
                                           imagePositions,
                                           16.0f, // layoutTextSize
                                           16.0f, // layoutTextSizeAtBucketZoomLevel
                                           /*allowVerticalPlacement*/ false);
        std::vector<std::size_t> counts;
        for (const auto& line : shaping.positionedLines) {
            counts.push_back(line.positionedGlyphs.size());
        }
        return counts;
    };

    // The break goes before the parenthesis, so it opens the second line instead of closing the first.
    EXPECT_EQ(glyphsPerLine(u"oooooooo(aaaaaaaaa)"), (std::vector<std::size_t>{8, 11}));
    EXPECT_EQ(glyphsPerLine(u"oooooooo (aaaaaaaaa)"), (std::vector<std::size_t>{8, 11}));
    // A trailing parenthesis never becomes a line of its own.
    EXPECT_EQ(glyphsPerLine(u"oooooooo("), (std::vector<std::size_t>{9}));
}

TEST(Shaping, applyTextFit) {
    float textSize = 4;
    float fontScale = 4;
    float expectedImageSize = textSize * fontScale;
    Shaping shapedText;
    setupShapedText(shapedText, textSize);

    {
        // applyTextFitHorizontal
        // This set of tests against applyTextFit starts with a 100x20 image with a 5,5,95,15 content box
        // that has been fitted to a 4*4 text with scale 4, resulting in a 16*16 image.
        const auto horizontalRectangle = Rect<uint16_t>(0, 0, 100, 20);
        const style::ImageContent horizontalContent = {.left = 5, .top = 5, .right = 95, .bottom = 15};

        {
            // applyTextFit: not specified
            // No change should happen
            testApplyTextFit(horizontalRectangle,
                             horizontalContent,
                             std::nullopt,
                             std::nullopt,
                             shapedText,
                             fontScale,
                             expectedImageSize,
                             expectedImageSize);
        }

        {
            // applyTextFit: both stretchOrShrink
            // No change should happen
            testApplyTextFit(horizontalRectangle,
                             horizontalContent,
                             style::TextFit::stretchOrShrink,
                             style::TextFit::stretchOrShrink,
                             shapedText,
                             fontScale,
                             expectedImageSize,
                             expectedImageSize);
        }

        {
            // applyTextFit: stretchOnly, proportional
            // Since textFitWidth is stretchOnly, it should be returned to
            // the aspect ratio of the content rectangle (9:1) aspect ratio so 144x16.
            testApplyTextFit(horizontalRectangle,
                             horizontalContent,
                             style::TextFit::stretchOnly,
                             style::TextFit::proportional,
                             shapedText,
                             fontScale,
                             expectedImageSize * 9,
                             expectedImageSize);
        }
    }

    {
        // applyTextFitVertical
        // This set of tests against applyTextFit starts with a 20x100 image with a 5,5,15,95 content box
        // that has been fitted to a 4*4 text with scale 4, resulting in a 16*16 image.
        const auto verticalRectangle = Rect<uint16_t>(0, 0, 20, 100);
        const style::ImageContent verticalContent = {.left = 5, .top = 5, .right = 15, .bottom = 95};

        {
            // applyTextFit: stretchOnly, proportional
            // Since textFitWidth is stretchOnly, it should be returned to
            // the aspect ratio of the content rectangle (9:1) aspect ratio so 144x16.
            testApplyTextFit(verticalRectangle,
                             verticalContent,
                             style::TextFit::proportional,
                             style::TextFit::stretchOnly,
                             shapedText,
                             fontScale,
                             expectedImageSize,
                             expectedImageSize * 9);
        }
    }
}
