#include "harfbuzz_impl.hpp"

#include <hb-ft.h>

namespace mbgl {

namespace {
hb_language_t getDefaultLanguage() {
    static hb_language_t language = hb_language_get_default();
    return language;
}

hb_script_t getUnicodeScript(hb_codepoint_t u) {
    static hb_unicode_funcs_t *unicode_funcs;

    unicode_funcs = hb_unicode_funcs_get_default();

    /* Make combining marks inherit the script of their bases, regardless of
     * their own script.
     */
    if (hb_unicode_general_category(unicode_funcs, u) == HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK)
        return HB_SCRIPT_INHERITED;

    return hb_unicode_script(unicode_funcs, u);
}

} // namespace

HBShaper::Impl::Impl(GlyphIDType type_, const std::string &fontFileData, const FreeTypeLibrary &lib)
    : type(type_),
      face(fontFileData.data(), fontFileData.size(), lib) {
    if (!face.isValid()) return;

    font = hb_ft_font_create(face.getFace(), NULL);
    buffer = hb_buffer_create();

    hb_buffer_allocation_successful(buffer);
}

HBShaper::Impl::~Impl() {
    if (!face.isValid()) return;
    hb_buffer_destroy(buffer);
    hb_font_destroy(font);
}
void HBShaper::Impl::createComplexGlyphIDs(const std::u16string &text,
                                           std::vector<GlyphID> &glyphIDs,
                                           std::vector<HBShapeAdjust> &adjusts) {
    if (text.empty()) {
        return;
    }

    struct TextPart {
        std::u16string text;
        hb_script_t script;
    };

    std::vector<TextPart> textParts;
    textParts.emplace_back();
    auto *lastTextPart = &textParts.back();
    lastTextPart->text = text[0];
    lastTextPart->script = getUnicodeScript(text[0]);

    for (std::size_t i = 1; i < text.size(); ++i) {
        auto ch = text[i];
        auto script = getUnicodeScript(text[i]);

        if (lastTextPart->script == script || script == HB_SCRIPT_INHERITED) {
            lastTextPart->text += (ch);
        } else {
            textParts.emplace_back();
            lastTextPart = &textParts.back();
            lastTextPart->text = (ch);
            lastTextPart->script = script;
        }
    }

    for (auto &textPart : textParts) {
        // Setup harfbuzz
        hb_buffer_reset(buffer);

        hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
        hb_buffer_set_script(buffer, textPart.script);
        hb_buffer_set_language(buffer, getDefaultLanguage());
        size_t length = textPart.text.size();

        hb_buffer_add_utf16(buffer, (const uint16_t *)textPart.text.c_str(), (int)length, 0, (int)length);

        // harfbuzz shaping
        hb_shape(font, buffer, NULL, 0);

        // Get Harfbuzz adjustion
        uint32_t glyphCount;
        hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
        hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);

        glyphIDs.reserve(glyphCount);
        adjusts.reserve(glyphCount);

        for (uint32_t i = 0; i < glyphCount; ++i) {
            glyphIDs.emplace_back(glyphInfo[i].codepoint, type);

            float x_advance = static_cast<float>(glyphPos[i].x_advance / 64.0f);
            float x_offset = static_cast<float>(glyphPos[i].x_offset / 64.0f);
            float y_offset = static_cast<float>(-glyphPos[i].y_offset / 64.0f);

            adjusts.emplace_back(x_offset, y_offset, x_advance);
        }
    }
}

} // namespace mbgl
