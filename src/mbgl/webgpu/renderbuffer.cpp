#include <mbgl/webgpu/renderbuffer.hpp>

namespace mbgl {
namespace webgpu {

RenderableResource::RenderableResource() = default;
RenderableResource::~RenderableResource() = default;

void RenderableResource::bind() {
    // TODO: Bind WebGPU framebuffer/render target
}

} // namespace webgpu
} // namespace mbgl