// GPU fences work fine with EGL, GLX and WGL but I noticed a crash when using QT
// This test is disabled for QT until the issue is resolved
#if MLN_RENDER_BACKEND_OPENGL && !defined(__QT__)

#include <mbgl/test/util.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/headless_backend.hpp>
#include <mbgl/gl/fence.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/gl/types.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>

using namespace mbgl;

// Free threaded resources are currrently only supported with AndroidRendererBackend
// All other backends require GL commands to be issued from a single thread
// In order to test asynchronous resources with HeadlessBackend, this test mocks shared
// context by running asynchronous operations using a ResourceUploadThreadPool but ensures
// all operation are serialized, i.e. only one backend scope is active at a time

// This mutex ensures one backend scope is active at any given time
// It is locked when a BackendScope is created and unlocked when it is destroyed
std::mutex& globalMutex() {
    static std::mutex mutex;
    return mutex;
}

class BackendWithMockedSharedContexts : public gl::HeadlessBackend {
public:
    BackendWithMockedSharedContexts()
        : gl::HeadlessBackend({32, 32}) {}

    bool supportFreeThreadedUpload() const override { return true; }

    std::shared_ptr<gl::UploadThreadContext> createUploadThreadContext() override;
};

class MockedUploadThreadContext : public gl::UploadThreadContext {
public:
    MockedUploadThreadContext(BackendWithMockedSharedContexts& backend_)
        : backend(backend_) {}
    ~MockedUploadThreadContext() override = default;
    void createContext() override {}
    void destroyContext() override {}
    void bindContext() override {
        globalMutex().lock();
        backendScopePtr = std::make_unique<gfx::BackendScope>(backend);
    }
    void unbindContext() override {
        MBGL_CHECK_ERROR(platform::glFinish());
        backendScopePtr.reset();
        globalMutex().unlock();
    }

private:
    BackendWithMockedSharedContexts& backend;
    std::unique_ptr<gfx::BackendScope> backendScopePtr = nullptr;
};

std::shared_ptr<gl::UploadThreadContext> BackendWithMockedSharedContexts::createUploadThreadContext() {
    return std::make_shared<MockedUploadThreadContext>(*this);
}

gl::BufferID createBufferId(gl::Fence& fence) {
    gl::BufferID id = 0;
    MBGL_CHECK_ERROR(platform::glGenBuffers(1, &id));
    EXPECT_GT(id, 0);
    fence.insert();
    return id;
}

void updateMemStats(gl::Context& context, const gl::VertexBufferResource& resource) {
    context.renderingStats().memVertexBuffers += resource.getByteSize();
    context.renderingStats().numBuffers++;
}

void updateMemStats(gl::Context& context, const gl::IndexBufferResource& resource) {
    context.renderingStats().memIndexBuffers += resource.getByteSize();
    context.renderingStats().numBuffers++;
}

template <typename Resource>
void testResource(BackendWithMockedSharedContexts& backend, gl::Context& context) {
    constexpr int data = 0;
    gl::Fence fence;

    auto buffer = std::make_unique<Resource>(
        [&](int, gfx::BufferUsageType, const void*) { return gl::UniqueBuffer{createBufferId(fence), {context}}; },
        [&](const gl::UniqueBuffer&, int, const void*) {
            fence.gpuWait(); // Expected to never block in the GPU
        },
        0);

    buffer->asyncAlloc(backend.getResourceUploadThreadPool(), sizeof(data), gfx::BufferUsageType::StaticDraw, &data);
    updateMemStats(context, *buffer);
    buffer->wait();

    buffer->asyncUpdate(backend.getResourceUploadThreadPool(), sizeof(data), &data);
    buffer->wait();

    {
        std::lock_guard<std::mutex> lock{globalMutex()};
        gfx::BackendScope scope{backend};
        EXPECT_TRUE(fence.isSignaled());
        fence.cpuWait(); // Expected to immediately return
    }
}

TEST(AsyncResources, AsyncResources) {
    BackendWithMockedSharedContexts backend;
    gl::Context context{backend};
    testResource<gl::VertexBufferResource>(backend, context);
    testResource<gl::IndexBufferResource>(backend, context);
}

#endif
