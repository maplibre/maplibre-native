#include <mbgl/gfx/drawable.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/renderer/render_pass.hpp>

namespace mbgl {
namespace gfx {

struct Drawable::Impl {
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
};

Drawable::Drawable(std::string name_)
    : name(name_),
      renderPass(RenderPass::Opaque),
      depthType(DepthMaskType::ReadOnly),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() = default;

const gfx::CullFaceMode& Drawable::getCullFaceMode() const {
    return impl->cullFaceMode;
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

void Drawable::setTexture(std::shared_ptr<gfx::Texture2D>& texture, int32_t location) {
    for (auto& tex : textures) {
        if (tex.location == location) {
            tex.texture = texture;
            return;
        }
    }
    textures.emplace_back(texture, location);
}

void Drawable::removeTexture(int32_t location) {
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        if (it->location == location) {
            textures.erase(it);
            return;
        }
    }
}

} // namespace gfx
} // namespace mbgl
