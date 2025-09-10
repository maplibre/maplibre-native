#include <mbgl/webgpu/render_target.hpp>

namespace mbgl {
namespace gfx {

RenderTarget::RenderTarget(Size size_, gfx::TextureChannelDataType type_)
    : size(size_), type(type_) {
    // TODO: Create WebGPU render target resources
}

RenderTarget::~RenderTarget() = default;

} // namespace gfx
} // namespace mbgl