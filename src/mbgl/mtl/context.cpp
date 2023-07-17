#include <mbgl/mtl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#endif

namespace mbgl {
namespace mtl {

Context::Context(RendererBackend& backend_)
    : gfx::Context(16), // TODO
      backend(backend_),
      stats() {}

Context::~Context() noexcept {
/*
    if (cleanupOnDestruction) {
        reset();
        assert(stats.isZero());
    }
 */
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<mtl::CommandEncoder>(*this);
}

void Context::performCleanup() {
}

void Context::reduceMemoryUsage() {
}

#if MLN_DRAWABLE_RENDERER
gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    assert(false);
    return nullptr;
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size) {
    assert(false);
    return nullptr;
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry&, const std::string& name) {
    assert(false);
    return nullptr;
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    assert(false);
    return nullptr;
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    assert(false);
    return nullptr;
}

gfx::Texture2DPtr Context::createTexture2D() {
    assert(false);
    return nullptr;
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    assert(false);
    return nullptr;
}

void Context::resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) {
}

#endif // MLN_DRAWABLE_RENDERER

void Context::setDirtyState() {
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size, gfx::TextureChannelDataType) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::TextureResource> Context::createTextureResource(Size,
                                                         gfx::TexturePixelType,
                                                         gfx::TextureChannelDataType) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                   Size size) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    assert(false);
    return nullptr;
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
}

void Context::visualizeDepthBuffer(float depthRangeSize) {
}
#endif // !defined(NDEBUG)

void Context::clearStencilBuffer(int32_t) {
}

} // namespace mtl
} // namespace mbgl

