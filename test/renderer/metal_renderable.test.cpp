#if MLN_RENDER_BACKEND_METAL
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/gfx/offscreen_texture.hpp>
#include <mln/mtl/context.hpp>
#include <mln/mtl/renderable_resource.hpp>
#include <mln/mtl/renderer_backend.hpp>

#include <Metal/Metal.hpp>

#include <chrono>
#include <thread>

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

// An offscreen pass must be committed without waiting for the GPU. Hold the queue on an event that only
// the CPU signals, a second after the pass is committed: if swap waited, it would come back after that
// second. Reading the image back still has to wait for the pass and return what it drew.
TEST(MetalRenderable, AnOffscreenPassDoesNotWaitForTheGPU) {
    auto backend = gfx::HeadlessBackend::Create({64, 64});
    gfx::BackendScope scope{*backend->getRendererBackend()};
    auto& rendererBackend = static_cast<mtl::RendererBackend&>(*backend->getRendererBackend());
    auto offscreen = rendererBackend.getContext<mtl::Context>().createOffscreenTexture(
        {64, 64}, gfx::TextureChannelDataType::UnsignedByte, false, false);
    auto& resource = offscreen->getResource<mtl::RenderableResource>();

    const auto event = NS::TransferPtr(rendererBackend.getDevice()->newSharedEvent());
    const auto blocker = rendererBackend.getCommandQueue()->commandBuffer();
    blocker->encodeWait(event.get(), 1);
    blocker->commit();
    std::thread release([&] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        event->setSignaledValue(1);
    });

    resource.bind();
    const auto colorTarget = resource.getRenderPassDescriptor()->colorAttachments()->object(0);
    colorTarget->setLoadAction(MTL::LoadActionClear);
    colorTarget->setStoreAction(MTL::StoreActionStore);
    colorTarget->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));
    resource.getCommandBuffer()->renderCommandEncoder(resource.getRenderPassDescriptor().get())->endEncoding();

    const auto start = std::chrono::steady_clock::now();
    resource.swap();
    const auto swapTime = std::chrono::steady_clock::now() - start;
    EXPECT_LT(swapTime, std::chrono::milliseconds(500));

    const PremultipliedImage image = offscreen->readStillImage();
    release.join();
    ASSERT_TRUE(image.valid());
    EXPECT_EQ(255, image.data[0]);
    EXPECT_EQ(0, image.data[1]);
    EXPECT_EQ(0, image.data[2]);
    EXPECT_EQ(255, image.data[3]);
}
#endif
