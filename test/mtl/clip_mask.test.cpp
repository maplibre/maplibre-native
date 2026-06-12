#if MLN_RENDER_BACKEND_METAL

#include <mbgl/test/util.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/command_encoder.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/headless_backend.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>

#include <array>
#include <vector>

using namespace mbgl;

namespace {

shaders::ClipUBO makeClipUBO(float scale, std::uint32_t stencilRef) {
    shaders::ClipUBO ubo{};
    // A simple scaled identity is enough: the test never reads pixels, it
    // only verifies which buffer the encoder was given.
    ubo.matrix = {scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1};
    ubo.stencil_ref = stencilRef;
    return ubo;
}

} // namespace

// Regression test for clip masks being encoded with a STALE UBO buffer.
//
// The second and subsequent renderTileClippingMasks calls within a frame
// keep their ClipUBO data in a stack-local temporary BufferResource.
// Consecutive rebuilds reallocate that temporary at the same stack address
// with the same initial version, so RenderPass's vertex-bind cache
// (`bind->buf == &buf && !buf.needReBind(bind->version)`) took a
// dangling-pointer cache hit and skipped the bind when the offset also
// matched (which it does whenever the previous rebuild contained a single
// tile). The skipped rebuild's masks were then encoded with the previous
// rebuild's tile matrices and stencil refs, so every stencil-clipped fill
// in that layer group vanished in tile-aligned regions that shifted with
// small camera moves.
//
// The single-tile / single-tile sequence below reproduces the exact
// collision: call 1 uses the persistent clip UBO buffer, calls 2 and 3 use
// the recycled stack temporary with bind offset 0. Call 3's bind must
// reach the encoder.
TEST(MetalClipMask, RebuildBindsFreshBufferAfterAddressReuse) {
    if (gfx::Backend::GetType() != gfx::Backend::Type::Metal) {
        return;
    }

    mtl::HeadlessBackend backend({64, 64});
    gfx::BackendScope scope{backend};
    auto& context = static_cast<mtl::Context&>(backend.getContext());

    RenderStaticData staticData{std::make_unique<gfx::ShaderRegistry>()};
    backend.initShaders(*staticData.shaders, ProgramParameters{1.0f, false});

    const auto encoder = context.createCommandEncoder();
    const gfx::RenderPassDescriptor descriptor{
        .renderable = backend.getDefaultRenderable(),
        .clearColor = Color::black(),
        .clearDepth = 1.0f,
        .clearStencil = 0,
    };
    const auto renderPass = encoder->createRenderPass("clip-mask-test", descriptor);

    const std::vector<shaders::ClipUBO> first = {makeClipUBO(1.0f, 1)};
    const std::vector<shaders::ClipUBO> second = {makeClipUBO(2.0f, 2)};
    const std::vector<shaders::ClipUBO> third = {makeClipUBO(3.0f, 3)};

    // Call 1: first rebuild of the "frame", uses the persistent buffer.
    ASSERT_TRUE(context.renderTileClippingMasks(*renderPass, staticData, first));
    // Call 2: first stack-temporary rebuild; binds fresh (the persistent
    // buffer's address is cached, the temporary's differs).
    ASSERT_TRUE(context.renderTileClippingMasks(*renderPass, staticData, second));

    // Call 3: the temporary is recycled at the same stack address as call
    // 2's (identical call path), with the same initial version and the
    // same bind offset (0, single tile). Without the unbind in
    // renderTileClippingMasks, the bind cache skips the encoder call and
    // these masks are encoded with call 2's UBO data.
    const auto bindsBefore = context.renderingStats().numVertexBufferBinds;
    ASSERT_TRUE(context.renderTileClippingMasks(*renderPass, staticData, third));
    const auto bindsAfter = context.renderingStats().numVertexBufferBinds;

    EXPECT_GT(bindsAfter, bindsBefore)
        << "the third clip-mask rebuild was encoded with the previous rebuild's "
           "UBO buffer (stale tile matrix + stencil ref): the vertex-bind cache "
           "took a dangling-pointer cache hit on the recycled stack temporary";
}

#endif // MLN_RENDER_BACKEND_METAL
