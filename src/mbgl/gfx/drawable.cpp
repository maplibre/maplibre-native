#include <mbgl/gfx/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/renderer/render_pass.hpp>

namespace mbgl {
namespace gfx {

int Drawable::bindUBOCount = 0;
int Drawable::bindUBOExecutedCount = 0;
int Drawable::bindUBOCacheHitCount = 0;

struct Drawable::Impl {
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
};

Drawable::Drawable(std::string name_)
    : name(name_),
      renderPass(RenderPass::Opaque),
      depthType(DepthMaskType::ReadOnly),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() = default;

const gfx::ColorMode& Drawable::getColorMode() const {
    return impl->colorMode;
}

void Drawable::setColorMode(const gfx::ColorMode& value) {
    impl->colorMode = value;
}

const gfx::CullFaceMode& Drawable::getCullFaceMode() const {
    return impl->cullFaceMode;
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

void Drawable::setIndexData(std::vector<std::uint16_t> indexes, std::vector<UniqueDrawSegment> segments) {
    setIndexData(std::make_shared<gfx::IndexVectorBase>(std::move(indexes)), std::move(segments));
}

static const gfx::Texture2DPtr noTexture;

const gfx::Texture2DPtr& Drawable::getTexture(int32_t location) const {
    const auto hit = textures.find(location);
    return (hit != textures.end()) ? hit->second : noTexture;
}

void Drawable::setTexture(std::shared_ptr<gfx::Texture2D> texture, int32_t location) {
    textures.insert(std::make_pair(location, gfx::Texture2DPtr{})).first->second = std::move(texture);
}

void Drawable::removeTexture(int32_t location) {
    textures.erase(location);
}

} // namespace gfx
} // namespace mbgl
