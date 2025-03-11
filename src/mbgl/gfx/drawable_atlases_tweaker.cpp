#include <mbgl/gfx/drawable_atlases_tweaker.hpp>

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/dynamic_texture.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/renderer/paint_parameters.hpp>

namespace mbgl {
namespace gfx {

void DrawableAtlasesTweaker::setupTextures(gfx::Drawable& drawable, const bool linearFilterForIcons) {
    if (const auto& shader = drawable.getShader()) {
        if (glyphTextureId) {
            if (iconTextureId && shader->getSamplerLocation(*iconTextureId)) {
                assert(*glyphTextureId != *iconTextureId);
                drawable.setTexture(atlases ? gfx::Context::getDynamicTextureAlpha()->getTextureAtlas() : nullptr,
                                    *glyphTextureId);
                drawable.setTexture(atlases ? gfx::Context::getDynamicTextureRGBA()->getTextureAtlas() : nullptr,
                                    *iconTextureId);
            } else {
                drawable.setTexture(atlases ? (isText ? gfx::Context::getDynamicTextureAlpha()->getTextureAtlas()
                                                      : gfx::Context::getDynamicTextureRGBA()->getTextureAtlas())
                                            : nullptr,
                                    *glyphTextureId);
            }
        }
    }
}

void DrawableAtlasesTweaker::init(gfx::Drawable& drawable) {
    setupTextures(drawable, true);
}

void DrawableAtlasesTweaker::execute(gfx::Drawable& drawable, PaintParameters& parameters) {
    const bool transformed = rotationAlignment == style::AlignmentType::Map || parameters.state.getPitch() != 0;
    const bool linearFilterForIcons = isText ? (parameters.state.isChanging() || transformed || !textSizeIsZoomConstant)
                                             : (sdfIcons || parameters.state.isChanging() || iconScaled || transformed);

    setupTextures(drawable, linearFilterForIcons);
}

} // namespace gfx
} // namespace mbgl
