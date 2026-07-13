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
                                  GlyphIDType type,
                                  bool keySection,
                                  std::optional<Color> textColor) {
    styledText.first += sectionText;
    auto startIndex = static_cast<uint32_t>(styledText.first.size());
    sections.emplace_back(scale, fontStack, type, startIndex, std::move(textColor));
    styledText.second.resize(styledText.first.size(), static_cast<uint8_t>(sections.size() - 1));
    supportsVerticalWritingMode = std::nullopt;
    if (type != GlyphIDType::FontPBF) hasNeedShapeTextVal = true;
    sections[sections.size() - 1].keySection = keySection;
}

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

    for (size_t i = 0; (i < beginningWhitespace) && i < styledText.first.length(); ++i) {
        auto &sec = getSection(i);
        if (sec.type != FontPBF) {
            beginningWhitespace = i;
            break;
        }
    }

    if (beginningWhitespace == std::u16string::npos) {
        for (auto &section : sections) {
            section.startIndex = 0;
        }
        // Entirely whitespace
        styledText.first.clear();
        styledText.second.clear();
    } else {
        int trailingWhitespace = static_cast<int>(styledText.first.find_last_not_of(u" \t\n\v\f\r") + 1);

        if (beginningWhitespace) {
            for (auto &section : sections) {
                section.startIndex -= beginningWhitespace;
            }
        }

        for (int i = static_cast<int>(styledText.first.length()) - 1; i >= trailingWhitespace; --i) {
            auto &sec = getSection(i);
            if (sec.type != FontPBF) {
                trailingWhitespace = i + 1;
                break;
            }
        }

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
    auto replaced = util::i18n::verticalizePunctuation(styledText.first);
    for (size_t i = 0; i < replaced.length(); ++i) {
        auto &sec = getSection(i);
        if (sec.type != GlyphIDType::FontPBF) replaced[i] = styledText.first[i];
    }
    styledText.first = replaced;
}

bool TaggedString::allowsVerticalWritingMode() {
    if (!supportsVerticalWritingMode) {
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
    }
    return *supportsVerticalWritingMode;
}

} // namespace mbgl
