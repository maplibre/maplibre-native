#if MLN_RENDER_BACKEND_METAL
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/mtl/renderable_resource.hpp>

#include <Metal/Metal.hpp>

using namespace mln;

// Each frame an upload pass and then a render pass bind the headless renderable, and only the
// swap after the render pass commits. If the second bind replaces the pending command buffer, one
// buffer per frame is never committed, and after about 64 frames (the queue's default pool) the
// next commandBuffer() call blocks forever.
TEST(MetalRenderable, BindKeepsThePendingCommandBuffer) {
    auto backend = gfx::HeadlessBackend::Create({256, 256});
    gfx::BackendScope scope{*backend->getRendererBackend()};
    auto& resource = backend->getRendererBackend()->getDefaultRenderable().getResource<mtl::RenderableResource>();

    resource.bind();
    const MTL::CommandBuffer* pending = resource.getCommandBuffer().get();
    ASSERT_NE(pending, nullptr);

    resource.bind();
    EXPECT_EQ(resource.getCommandBuffer().get(), pending);

    resource.swap();
    resource.bind();
    EXPECT_NE(resource.getCommandBuffer().get(), nullptr);
    resource.swap();
}
#endif
