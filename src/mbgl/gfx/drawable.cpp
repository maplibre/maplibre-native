#include <mbgl/gfx/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_impl.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/scissor_rect.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/render_pass.hpp>

namespace mbgl {
namespace gfx {

struct Drawable::Impl {
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
    gfx::ScissorRect scissorRect = {0, 0, 0, 0};

    std::shared_ptr<Bucket> bucket;
    PaintPropertyBindersBase* binders = nullptr; // owned by `bucket`

    Immutable<std::vector<RenderTile>> renderTiles = makeMutable<std::vector<RenderTile>>();
    const RenderTile* renderTile = nullptr; // owned by `renderTiles`
};

Drawable::Drawable(std::string name_)
    : name(name_),
      renderPass(mbgl::RenderPass::Opaque),
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

const gfx::ScissorRect& Drawable::getScissorRect() const {
    return impl->scissorRect;
}

void Drawable::setScissorRect(const gfx::ScissorRect& value) {
    impl->scissorRect = value;
}

void Drawable::setIndexData(std::vector<std::uint16_t> indexes, std::vector<UniqueDrawSegment> segments) {
    setIndexData(std::make_shared<gfx::IndexVectorBase>(std::move(indexes)), std::move(segments));
}

static const gfx::Texture2DPtr noTexture;

const gfx::Texture2DPtr& Drawable::getTexture(size_t id) const {
    return (id < textures.size()) ? textures[id] : noTexture;
}

void Drawable::setTexture(std::shared_ptr<gfx::Texture2D> texture, size_t id) {
    assert(id < textures.size());
    if (id >= textures.size()) {
        return;
    }
    textures[id] = std::move(texture);
}

PaintPropertyBindersBase* Drawable::getBinders() {
    return impl->binders;
}
const PaintPropertyBindersBase* Drawable::getBinders() const {
    return impl->binders;
}

/// Set the property binders used for property updates
void Drawable::setBinders(std::shared_ptr<Bucket> bucket_, PaintPropertyBindersBase* binders_) {
    impl->bucket = std::move(bucket_);
    impl->binders = binders_;
}

const RenderTile* Drawable::getRenderTile() const {
    return impl->renderTile;
}

const std::shared_ptr<Bucket>& Drawable::getBucket() const {
    return impl->bucket;
}

void Drawable::setRenderTile(Immutable<std::vector<RenderTile>> renderTiles_, const RenderTile* tile_) {
    impl->renderTiles = std::move(renderTiles_);
    impl->renderTile = tile_;
}

} // namespace gfx
} // namespace mbgl
