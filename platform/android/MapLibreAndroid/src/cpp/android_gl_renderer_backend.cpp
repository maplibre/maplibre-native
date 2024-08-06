#include "android_gl_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>

#include <sys/system_properties.h>

#include <cassert>
#include <mutex>
#include <stdexcept>

namespace mbgl {
namespace android {
namespace {

std::mutex& getEglMutex() {
    static std::mutex eglMutex;
    return eglMutex;
}

std::string androidSysProp(const char* key) {
    assert(strlen(key) < PROP_NAME_MAX);
    if (__system_property_find(key) == nullptr) {
        return "";
    }
    char prop[PROP_VALUE_MAX + 1];
    __system_property_get(key, prop);
    return prop;
}

bool inEmulator() {
    return androidSysProp("ro.kernel.qemu") == "1" || androidSysProp("ro.boot.qemu") == "1" ||
           androidSysProp("ro.hardware.egl") == "emulation" ||
           util::contains(androidSysProp("ro.build.fingerprint"), "emu") ||
           util::contains(androidSysProp("ro.build.product"), "emu") ||
           util::contains(androidSysProp("ro.product.device"), "emu");
}

} // namespace

class AndroidGLRenderableResource final : public mbgl::gl::RenderableResource {
public:
    AndroidGLRenderableResource(AndroidGLRendererBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        assert(gfx::BackendScope::exists());
        backend.setFramebufferBinding(0);
        backend.setViewport(0, 0, backend.getSize());
    }

    void swap() override {
        const auto& swapBehaviour = static_cast<AndroidGLRendererBackend&>(backend).getSwapBehavior();
        if (swapBehaviour == gfx::Renderable::SwapBehaviour::Flush) {
            static_cast<gl::Context&>(backend.getContext()).finish();
        }
    }

private:
    AndroidGLRendererBackend& backend;
};

AndroidGLRendererBackend::AndroidGLRendererBackend(bool multiThreadedGpuResourceUpload_)
    : gl::RendererBackend(gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({64, 64}, std::make_unique<AndroidGLRenderableResource>(*this)) {
    if (multiThreadedGpuResourceUpload_) {
        if (inEmulator()) {
            mbgl::Log::Error(mbgl::Event::OpenGL,
                             "Multi-threaded GPU resource upload is not supported in the emulator");
        } else {
            multiThreadedGpuResourceUpload = true;
        }
    }
}

AndroidGLRendererBackend::~AndroidGLRendererBackend() {
    if (eglWaitClient() == EGL_FALSE) {
        auto err = "eglWaitClient failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }
    destroyResourceUploadThreadPool();
}

gl::ProcAddress AndroidGLRendererBackend::getExtensionFunctionPointer(const char* name) {
    assert(gfx::BackendScope::exists());
    return eglGetProcAddress(name);
}

void AndroidGLRendererBackend::updateViewPort() {
    assert(gfx::BackendScope::exists());
    setViewport(0, 0, size);
}

void AndroidGLRendererBackend::resizeFramebuffer(int width, int height) {
    size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

PremultipliedImage AndroidGLRendererBackend::readFramebuffer() {
    assert(gfx::BackendScope::exists());
    return gl::RendererBackend::readFramebuffer(size);
}

void AndroidGLRendererBackend::updateAssumedState() {
    assumeFramebufferBinding(0);
    assumeViewport(0, 0, size);
}

void AndroidGLRendererBackend::markContextLost() {
    if (context) {
        getContext<gl::Context>().setCleanupOnDestruction(false);
    }
}

bool AndroidGLRendererBackend::supportFreeThreadedUpload() const {
    return multiThreadedGpuResourceUpload;
}

std::shared_ptr<gl::UploadThreadContext> AndroidGLRendererBackend::createUploadThreadContext() {
    MLN_TRACE_FUNC();

    assert(eglMainCtx != EGL_NO_CONTEXT);
    return std::make_shared<AndroidUploadThreadContext>(*this, eglDsply, eglConfig, eglMainCtx, eglClientVersion);
}

void AndroidGLRendererBackend::initFreeThreadedUpload() {
    MLN_TRACE_FUNC();

    if (eglMainCtx != EGL_NO_CONTEXT) {
        // Already initialized
        return;
    }
    assert(eglDsply == EGL_NO_DISPLAY);
    assert(eglSurf == EGL_NO_SURFACE);
    assert(eglGetError() == EGL_SUCCESS);

    eglMainCtx = eglGetCurrentContext();
    if (eglMainCtx == EGL_NO_CONTEXT) {
        constexpr const char* err = "eglGetCurrentContext returned EGL_NO_CONTEXT";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    eglDsply = eglGetCurrentDisplay();
    if (eglDsply == EGL_NO_DISPLAY) {
        constexpr const char* err = "eglGetCurrentDisplay returned EGL_NO_DISPLAY";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    eglSurf = eglGetCurrentSurface(EGL_READ);
    if (eglSurf == EGL_NO_SURFACE) {
        constexpr const char* err = "eglGetCurrentSurface returned EGL_NO_SURFACE";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    EGLSurface writeSurf = eglGetCurrentSurface(EGL_DRAW);
    if (eglSurf != writeSurf) {
        constexpr const char* err = "EGL_READ and EGL_DRAW surfaces are different";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    int config_id = 0;
    if (eglQueryContext(eglDsply, eglMainCtx, EGL_CONFIG_ID, &config_id) == EGL_FALSE) {
        auto err = "eglQueryContext for EGL_CONFIG_ID failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    int config_count = 0;
    const EGLint attribs[] = {EGL_CONFIG_ID, config_id, EGL_NONE};
    if (eglChooseConfig(eglDsply, attribs, nullptr, 0, &config_count) == EGL_FALSE) {
        auto err = "eglChooseConfig failed to query config_count. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }
    if (config_count != 1) {
        auto err = "eglChooseConfig returned multiple configs: " + std::to_string(config_count);
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    if (eglChooseConfig(eglDsply, attribs, &eglConfig, 1, &config_count) == EGL_FALSE) {
        auto err = "eglChooseConfig failed to query config. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    if (eglQueryContext(eglDsply, eglMainCtx, EGL_CONTEXT_CLIENT_VERSION, &eglClientVersion) == EGL_FALSE) {
        auto err = "eglQueryContext for client version failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }
}

AndroidUploadThreadContext::AndroidUploadThreadContext(AndroidRendererBackend& backend_,
                                                       EGLDisplay display_,
                                                       EGLConfig config_,
                                                       EGLContext mainContext_,
                                                       int clientVersion_)
    : backend(backend_),
      display(display_),
      config(config_),
      mainContext(mainContext_),
      clientVersion(clientVersion_) {}

AndroidUploadThreadContext::~AndroidUploadThreadContext() {
    MLN_TRACE_FUNC();

    auto ctx = eglGetCurrentContext();
    if (ctx == EGL_NO_CONTEXT) {
        return; // Upload thread clean from any EGL context
    }

    if (ctx == sharedContext) {
        mbgl::Log::Error(mbgl::Event::OpenGL, "AndroidUploadThreadContext::destroyContext() must be explicitly called");
    } else {
        mbgl::Log::Error(mbgl::Event::OpenGL, "Unexpected context bound to an Upload thread");
    }
    assert(ctx == EGL_NO_CONTEXT);
}

void AndroidUploadThreadContext::createContext() {
    MLN_TRACE_FUNC();

    const std::lock_guard<std::mutex> lock(getEglMutex());

    assert(display != EGL_NO_DISPLAY);
    assert(mainContext != EGL_NO_CONTEXT);
    assert(sharedContext == EGL_NO_CONTEXT);
    assert(surface == EGL_NO_SURFACE);

    int attribs[] = {EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE};
    sharedContext = eglCreateContext(display, config, mainContext, attribs);
    if (sharedContext == EGL_NO_CONTEXT) {
        auto err = "eglCreateContext returned EGL_NO_CONTEXT. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    surface = eglCreatePbufferSurface(display, config, nullptr);
    if (surface == EGL_NO_SURFACE) {
        auto err = "eglCreatePbufferSurface failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }

    if (eglMakeCurrent(display, surface, surface, sharedContext) == EGL_FALSE) {
        auto err = "eglMakeCurrent for shared context failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
        throw std::runtime_error(err);
    }
    MLN_TRACE_GL_CONTEXT();
}

void AndroidUploadThreadContext::destroyContext() {
    MLN_TRACE_FUNC();

    const std::lock_guard<std::mutex> lock(getEglMutex());

    if (eglWaitClient() == EGL_FALSE) {
        auto err = "eglWaitClient failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }

    auto ctx = eglGetCurrentContext();
    if (ctx == EGL_NO_CONTEXT) {
        constexpr const char* err =
            "AndroidUploadThreadContext::destroyContext() expects a persistently bound EGL shared context";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    } else if (ctx != sharedContext) {
        constexpr const char* err =
            "AndroidUploadThreadContext::destroyContext(): expects a single EGL context to be used in each Upload "
            "thread";
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }
    assert(ctx == sharedContext);

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE) {
        auto err = "eglMakeCurrent with EGL_NO_CONTEXT failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }
    if (eglDestroyContext(display, sharedContext) == EGL_FALSE) {
        auto err = "eglDestroyContext failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }
    if (eglDestroySurface(display, surface) == EGL_FALSE) {
        auto err = "eglDestroySurface failed. Error code " + std::to_string(eglGetError());
        mbgl::Log::Error(mbgl::Event::OpenGL, err);
    }

    display = EGL_NO_DISPLAY;
    mainContext = EGL_NO_CONTEXT;
    sharedContext = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}

void AndroidUploadThreadContext::bindContext() {
    // Expect a persistently bound EGL shared context
    assert(eglGetCurrentContext() == sharedContext && sharedContext != EGL_NO_CONTEXT);
}

void AndroidUploadThreadContext::unbindContext() {
    // Expect a persistently bound EGL shared context
    assert(eglGetCurrentContext() == sharedContext && sharedContext != EGL_NO_CONTEXT);
}

} // namespace android
} // namespace mbgl

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<android::AndroidRendererBackend> Backend::Create<mbgl::gfx::Backend::Type::OpenGL>(
    ANativeWindow*, bool multiThreadedGpuResourceUpload) {
    return std::make_unique<android::AndroidGLRendererBackend>(multiThreadedGpuResourceUpload);
}

} // namespace gfx
} // namespace mbgl