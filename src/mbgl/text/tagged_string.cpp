#include <mbgl/math/minmax.hpp>
#include <mbgl/text/tagged_string.hpp>
#include <mbgl/util/i18n.hpp>
#include <mbgl/util/logging.hpp>

namespace {
char16_t PUAbegin = u'\uE000';
char16_t PUAend = u'\uF8FF';
} // namespace

namespace mbgl {

void TaggedString::addTextSection(const std::u16string &sectionText,
                                  double scale,
                                  const FontStack &fontStack,
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
                                  GlyphIDType type,
                                  bool keySection,
#endif
                                  std::optional<Color> textColor) {
    styledText.first += sectionText;
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    auto startIndex = static_cast<uint32_t>(styledText.first.size());
    sections.emplace_back(scale, fontStack, type, startIndex, std::move(textColor));
#else
    sections.emplace_back(scale, fontStack, std::move(textColor));
#endif
    styledText.second.resize(styledText.first.size(), static_cast<uint8_t>(sections.size() - 1));
    supportsVerticalWritingMode = std::nullopt;
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    if (type != GlyphIDType::FontPBF) hasNeedShapeTextVal = true;
    sections[sections.size() - 1].keySection = keySection;
#endif
}

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
void TaggedString::addTextSection(const std::u16string &sectionText,
                                  double scale,
                                  const FontStack &fontStack,
                                  GlyphIDType type,
                                  std::shared_ptr<std::vector<HBShapeAdjust>> &adjusts,
                                  bool keySection,
                                  std::optional<Color> textColor) {
    sections.emplace_back(scale, fontStack, type, static_cast<uint32_t>(styledText.first.size()), std::move(textColor));
    styledText.first += sectionText;
    styledText.second.resize(styledText.first.size(), static_cast<uint8_t>(sections.size() - 1));
    if (type != GlyphIDType::FontPBF) hasNeedShapeTextVal = true;
    if (adjusts) sections[sections.size() - 1].adjusts = adjusts;
    sections[sections.size() - 1].keySection = keySection;
}
#endif

void TaggedString::addImageSection(const std::string &imageID) {
    const auto &nextImageSectionCharCode = getNextImageSectionCharCode();
    if (!nextImageSectionCharCode) {
        Log::Warning(Event::Style, "Exceeded maximum number of images in a label.");
        return;
    }

    styledText.first += *nextImageSectionCharCode;
    sections.emplace_back(imageID);
    styledText.second.resize(styledText.first.size(), static_cast<uint8_t>(sections.size() - 1));
}

std::optional<char16_t> TaggedString::getNextImageSectionCharCode() {
    if (imageSectionID == 0u) {
        imageSectionID = PUAbegin;
        return imageSectionID;
    }

    if (++imageSectionID > PUAend) {
        return std::nullopt;
    }

    return imageSectionID;
}

void TaggedString::trim() {
    std::size_t beginningWhitespace = styledText.first.find_first_not_of(u" \t\n\v\f\r");

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    for (size_t i = 0; (i < beginningWhitespace) && i < styledText.first.length(); ++i) {
        auto &sec = getSection(i);
        if (sec.type != FontPBF) {
            beginningWhitespace = i;
            break;
        }
    }
#endif

    if (beginningWhitespace == std::u16string::npos) {
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
        for (auto &section : sections) {
            section.startIndex = 0;
        }
#endif
        // Entirely whitespace
        styledText.first.clear();
        styledText.second.clear();
    } else {
        std::size_t trailingWhitespace = styledText.first.find_last_not_of(u" \t\n\v\f\r") + 1;

#ifdef MLN_TEXT_SHAPING_HARFBUZZ
        if (beginningWhitespace) {
            for (auto &section : sections) {
                section.startIndex -= beginningWhitespace;
            }
        }

        for (size_t i = styledText.first.length() - 1; i >= trailingWhitespace; --i) {
            auto &sec = getSection(i);
            if (sec.type != FontPBF) {
                trailingWhitespace = i + 1;
                break;
            }
        }
#endif

        styledText.first = styledText.first.substr(beginningWhitespace, trailingWhitespace - beginningWhitespace);
        styledText.second = std::vector<uint8_t>(styledText.second.begin() + beginningWhitespace,
                                                 styledText.second.begin() + trailingWhitespace);
    }
}

double TaggedString::getMaxScale() const {
    double maxScale = 0.0;
    for (std::size_t i = 0; i < styledText.first.length(); i++) {
        maxScale = util::max(maxScale, getSection(i).scale);
    }
    return maxScale;
}

void TaggedString::verticalizePunctuation() {
    // Relies on verticalization changing characters in place so that style indices don't need updating
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
    auto replaced = util::i18n::verticalizePunctuation(styledText.first);
    for (size_t i = 0; i < replaced.length(); ++i) {
        auto &sec = getSection(i);
        if (sec.type != GlyphIDType::FontPBF) replaced[i] = styledText.first[i];
    }
    styledText.first = replaced;
#else
    styledText.first = util::i18n::verticalizePunctuation(styledText.first);
#endif
}

bool TaggedString::allowsVerticalWritingMode() {
    if (!supportsVerticalWritingMode) {
#ifdef MLN_TEXT_SHAPING_HARFBUZZ
        bool allows = false;
        for (size_t i = 0; i < styledText.first.length(); ++i) {
            auto chr = styledText.first[i];
            auto &sec = getSection(i);
            if (sec.type == GlyphIDType::FontPBF && util::i18n::hasUprightVerticalOrientation(chr)) {
                allows = true;
                break;
            }
        }
        supportsVerticalWritingMode = allows;
#else
        supportsVerticalWritingMode = util::i18n::allowsVerticalWritingMode(rawText());
#endif
    }
    return *supportsVerticalWritingMode;
}

} // namespace mbgl
