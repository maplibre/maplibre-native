#include <mbgl/gfx/drawable.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {
namespace gfx {

struct Drawable::Impl {
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
};

Drawable::Drawable(std::string name_)
    : name(name_),
      renderPass(RenderPass::Opaque),
      matrix(matrix::identity4()),
      depthType(DepthMaskType::ReadOnly),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() = default;

const gfx::CullFaceMode& Drawable::getCullFaceMode() const {
    return impl->cullFaceMode;
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

} // namespace gfx
} // namespace mbgl
