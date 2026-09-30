#include <mln/test/util.hpp>
#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/util/run_loop.hpp>
#include <array>
#include <cstddef>
#include <memory>

#if MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_WEBGPU
#include <mln/gfx/command_encoder.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/upload_pass.hpp>
#if MLN_RENDER_BACKEND_METAL
#include <mln/mtl/vertex_buffer_resource.hpp>
#elif MLN_RENDER_BACKEND_VULKAN
#include <mln/vulkan/vertex_buffer_resource.hpp>
#else
#include <mln/webgpu/vertex_buffer_resource.hpp>
#endif
#endif

using namespace mln;

#if MLN_RENDER_BACKEND_METAL || MLN_RENDER_BACKEND_VULKAN || MLN_RENDER_BACKEND_WEBGPU

TEST(VertexBuffer, UploadTimestamp) {
#if MLN_RENDER_BACKEND_METAL
    using Resource = mtl::VertexBufferResource;
#elif MLN_RENDER_BACKEND_VULKAN
    using Resource = vulkan::VertexBufferResource;
#else
    using Resource = webgpu::VertexBufferResource;
#endif
    util::RunLoop loop;
    HeadlessFrontend frontend{1.0f};
    auto& backend = *frontend.getBackend();
    gfx::BackendScope scope{backend};
    auto encoder = backend.getContext().createCommandEncoder();
    auto upload = encoder->createUploadPass("vertex timestamp", backend.getDefaultRenderable());
    gfx::VertexVector<uint32_t> vertices;
    vertices.emplace_back(42);

    const auto before = util::MonotonicTimer::now();
    auto buffer = upload->createVertexBuffer(vertices);
    auto& resource = buffer.getResource<Resource>();
    EXPECT_GE(resource.getLastUpdated(), before);
    EXPECT_LE(resource.getLastUpdated(), util::MonotonicTimer::now());

    const auto uploaded = std::chrono::duration<double>{123};
    resource.setLastUpdated(uploaded);
    alignas(Resource) std::array<std::byte, sizeof(Resource)> storage;
    storage.fill(std::byte{0xff});
    auto* moved = std::construct_at(reinterpret_cast<Resource*>(storage.data()), std::move(resource));
    EXPECT_EQ(uploaded, moved->getLastUpdated());
    std::destroy_at(moved);
}
#endif
