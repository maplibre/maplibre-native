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

void Drawable::setTexture(std::shared_ptr<gfx::Texture2D> texture, int32_t location) {
    for (auto& tex : textures) {
        if (tex.location == location) {
            tex.texture = std::move(texture);
            return;
        }
    }
    textures.emplace_back(std::move(texture), location);
}

void Drawable::removeTexture(int32_t location) {
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        if (it->location == location) {
            textures.erase(it);
            return;
        }
    }
}

/// @brief Provide a function to get the current texture
void Drawable::setTextureSource(int32_t location, TexSourceFunc source) {
    textureSources.resize(std::max(textureSources.size(), static_cast<size_t>(location + 1)));
    textureSources[location] = std::move(source);
}

static const Drawable::TexSourceFunc noSource;

const Drawable::TexSourceFunc& Drawable::getTextureSource(int32_t location) const {
    return (static_cast<std::size_t>(location) < textureSources.size()) ? textureSources[location] : noSource;
}

/// @brief Provide all texture sources at once
void Drawable::setTextureSources(std::vector<TexSourceFunc> sources) {
    textureSources = std::move(sources);
}

} // namespace gfx
} // namespace mbgl
