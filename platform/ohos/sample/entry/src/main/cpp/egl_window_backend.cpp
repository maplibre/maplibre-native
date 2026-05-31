#include "egl_window_backend.hpp"

#include "native_window_utils.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/renderable_resource.hpp>
#include <mbgl/platform/gl_functions.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <GLES3/gl3.h>
#include <native_buffer/native_buffer.h>

#include <array>
#include <sstream>
#include <stdexcept>
#include <string>

namespace mbgl {
namespace ohos {
namespace {

void logDiagnostic(const std::string& message) {
    Log::Info(Event::OpenGL, message);
}

std::string eglErrorMessage(const char* operation) {
    std::ostringstream stream;
    stream << operation << " failed with EGL error 0x" << std::hex << eglGetError();
    return stream.str();
}

void setNativeWindowPixelFormat(OHNativeWindow* window) {
    if (window == nullptr) {
        return;
    }

    const auto format = static_cast<std::int32_t>(NATIVEBUFFER_PIXEL_FMT_RGBA_8888);
    if (OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, format) != 0) {
        Log::Warning(Event::OpenGL, "OH_NativeWindow_NativeWindowHandleOpt SET_FORMAT failed");
    }
}

void logNativeWindowFormat(OHNativeWindow* window) {
    if (window == nullptr) {
        return;
    }

    std::int32_t format = 0;
    if (OH_NativeWindow_NativeWindowHandleOpt(window, GET_FORMAT, &format) == 0) {
        logDiagnostic("Native window pixel format: " + std::to_string(format));
    }
}

struct PixelFormatPreference {
    EGLint red;
    EGLint green;
    EGLint blue;
    EGLint alpha;
};

EGLint getConfigAttribute(EGLDisplay display, EGLConfig config, EGLint attribute) {
    EGLint value = 0;
    if (!eglGetConfigAttrib(display, config, attribute, &value)) {
        return -1;
    }
    return value;
}

std::string glString(GLenum name) {
    const auto* value = platform::glGetString(name);
    return value ? reinterpret_cast<const char*>(value) : "";
}

std::string formatEGLConfig(EGLDisplay display, EGLConfig config, std::int32_t contextClientVersion) {
    std::ostringstream stream;
    stream << "EGL config"
           << " id=" << getConfigAttribute(display, config, EGL_CONFIG_ID)
           << " visual=" << getConfigAttribute(display, config, EGL_NATIVE_VISUAL_ID)
           << " rgba=" << getConfigAttribute(display, config, EGL_RED_SIZE) << '/'
           << getConfigAttribute(display, config, EGL_GREEN_SIZE) << '/'
           << getConfigAttribute(display, config, EGL_BLUE_SIZE) << '/'
           << getConfigAttribute(display, config, EGL_ALPHA_SIZE)
           << " depth=" << getConfigAttribute(display, config, EGL_DEPTH_SIZE)
           << " stencil=" << getConfigAttribute(display, config, EGL_STENCIL_SIZE) << " surface=0x" << std::hex
           << getConfigAttribute(display, config, EGL_SURFACE_TYPE) << " renderable=0x"
           << getConfigAttribute(display, config, EGL_RENDERABLE_TYPE) << std::dec << " es=" << contextClientVersion;
    return stream.str();
}

std::string formatDefaultFramebuffer() {
    platform::GLint red = 0;
    platform::GLint green = 0;
    platform::GLint blue = 0;
    platform::GLint alpha = 0;
    platform::GLint depth = 0;
    platform::GLint stencil = 0;
    platform::glGetIntegerv(GL_RED_BITS, &red);
    platform::glGetIntegerv(GL_GREEN_BITS, &green);
    platform::glGetIntegerv(GL_BLUE_BITS, &blue);
    platform::glGetIntegerv(GL_ALPHA_BITS, &alpha);
    platform::glGetIntegerv(GL_DEPTH_BITS, &depth);
    platform::glGetIntegerv(GL_STENCIL_BITS, &stencil);
    const auto framebufferStatus = platform::glCheckFramebufferStatus(GL_FRAMEBUFFER);
    const auto error = platform::glGetError();

    std::ostringstream stream;
    stream << "GL default framebuffer"
           << " rgba=" << red << '/' << green << '/' << blue << '/' << alpha << " depth=" << depth
           << " stencil=" << stencil << " status=0x" << std::hex << framebufferStatus << " err=0x" << error << std::dec
           << " vendor=" << glString(GL_VENDOR) << " renderer=" << glString(GL_RENDERER)
           << " version=" << glString(GL_VERSION);
    return stream.str();
}

bool tryCreateWindowSurface(EGLDisplay display,
                            OHNativeWindow* window,
                            EGLint renderableType,
                            EGLint contextVersion,
                            PixelFormatPreference format,
                            EGLConfig& chosenConfig,
                            EGLContext& chosenContext,
                            EGLSurface& chosenSurface) {
    const EGLint attributes[] = {
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,
        renderableType,
        EGL_RED_SIZE,
        format.red,
        EGL_GREEN_SIZE,
        format.green,
        EGL_BLUE_SIZE,
        format.blue,
        EGL_ALPHA_SIZE,
        format.alpha,
        EGL_DEPTH_SIZE,
        16,
        EGL_STENCIL_SIZE,
        8,
        EGL_NONE,
    };

    std::array<EGLConfig, 16> configs{};
    EGLint configCount = 0;
    if (!eglChooseConfig(display, attributes, configs.data(), configs.size(), &configCount) || configCount <= 0) {
        return false;
    }

    const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, contextVersion, EGL_NONE};
    for (EGLint index = 0; index < configCount; ++index) {
        EGLContext context = eglCreateContext(display, configs[index], EGL_NO_CONTEXT, contextAttributes);
        if (context == EGL_NO_CONTEXT) {
            continue;
        }

        EGLSurface surface = eglCreateWindowSurface(display, configs[index], window, nullptr);
        if (surface != EGL_NO_SURFACE) {
            chosenConfig = configs[index];
            chosenContext = context;
            chosenSurface = surface;
            return true;
        }

        eglDestroyContext(display, context);
        eglGetError();
    }

    return false;
}

bool createWindowEglState(EGLDisplay display,
                          OHNativeWindow* window,
                          EGLConfig& config,
                          EGLContext& context,
                          EGLSurface& surface,
                          std::int32_t& contextClientVersion) {
    static constexpr std::array<PixelFormatPreference, 3> formats{
        PixelFormatPreference{8, 8, 8, 8},
        PixelFormatPreference{8, 8, 8, 0},
        PixelFormatPreference{5, 6, 5, 0},
    };

    static constexpr std::array<std::pair<EGLint, EGLint>, 2> apiVersions{
        std::pair{EGL_OPENGL_ES3_BIT, 3},
        std::pair{EGL_OPENGL_ES2_BIT, 2},
    };

    for (const auto& apiVersion : apiVersions) {
        for (const auto& format : formats) {
            if (tryCreateWindowSurface(
                    display, window, apiVersion.first, apiVersion.second, format, config, context, surface)) {
                contextClientVersion = apiVersion.second;
                if (apiVersion.second == 2) {
                    Log::Warning(Event::OpenGL, "Using OpenGL ES 2 EGL window surface");
                }
                return true;
            }
        }
    }

    return false;
}

class EGLWindowRenderableResource final : public gl::RenderableResource {
public:
    explicit EGLWindowRenderableResource(EGLWindowBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        MLN_TRACE_FUNC();

        backend.setFramebufferBinding(0);
        backend.setViewport(0, 0, backend.getSize());
    }

    void swap() override {
        MLN_TRACE_FUNC();

        backend.swap();
    }

private:
    EGLWindowBackend& backend;
};

} // namespace

class EGLWindowBackend::DisplayConfig {
private:
    struct Key {
        explicit Key() = default;
    };

public:
    explicit DisplayConfig(Key) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            throw std::runtime_error("Failed to obtain a valid EGL display");
        }

        EGLint major = 0;
        EGLint minor = 0;
        if (!eglInitialize(display, &major, &minor)) {
            throw std::runtime_error(eglErrorMessage("eglInitialize"));
        }

        if (!eglBindAPI(EGL_OPENGL_ES_API)) {
            throw std::runtime_error(eglErrorMessage("eglBindAPI"));
        }
    }

    ~DisplayConfig() {
        if (display != EGL_NO_DISPLAY) {
            eglTerminate(display);
        }
    }

    static std::shared_ptr<const DisplayConfig> create() {
        static std::weak_ptr<const DisplayConfig> instance;
        auto shared = instance.lock();
        if (!shared) {
            instance = shared = std::make_shared<DisplayConfig>(Key{});
        }
        return shared;
    }

    EGLDisplay display = EGL_NO_DISPLAY;
};

EGLWindowBackend::EGLWindowBackend(OHNativeWindow* window_, Size size_)
    : gl::RendererBackend(gfx::ContextMode::Unique),
      gfx::Renderable(size_, std::make_unique<EGLWindowRenderableResource>(*this)),
      displayConfig(DisplayConfig::create()),
      window(window_) {
    MLN_TRACE_FUNC();

    if (window == nullptr) {
        throw std::invalid_argument("EGLWindowBackend requires a native window");
    }

    const auto referenceResult = OH_NativeWindow_NativeObjectReference(window);
    if (referenceResult != 0) {
        throw std::runtime_error("OH_NativeWindow_NativeObjectReference failed");
    }

    try {
        if (!setNativeWindowBufferGeometry(window, size_)) {
            Log::Warning(Event::OpenGL, "OH_NativeWindow_NativeWindowHandleOpt SET_BUFFER_GEOMETRY failed");
        }
        setNativeWindowPixelFormat(window);
        logNativeWindowFormat(window);

        EGLConfig windowConfig = nullptr;
        if (!createWindowEglState(
                displayConfig->display, window, windowConfig, eglContext, eglSurface, contextClientVersion)) {
            throw std::runtime_error(eglErrorMessage("eglCreateWindowSurface"));
        }
        logDiagnostic(formatEGLConfig(displayConfig->display, windowConfig, contextClientVersion));
    } catch (...) {
        if (eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(displayConfig->display, eglSurface);
            eglSurface = EGL_NO_SURFACE;
        }
        if (eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(displayConfig->display, eglContext);
            eglContext = EGL_NO_CONTEXT;
        }
        OH_NativeWindow_NativeObjectUnreference(window);
        window = nullptr;
        throw;
    }
}

EGLWindowBackend::~EGLWindowBackend() {
    MLN_TRACE_FUNC();

    if (eglContext != EGL_NO_CONTEXT && eglGetCurrentContext() == eglContext) {
        if (!eglMakeCurrent(displayConfig->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
            Log::Warning(Event::OpenGL, eglErrorMessage("eglMakeCurrent"));
        }
    }

    if (eglSurface != EGL_NO_SURFACE) {
        if (!eglDestroySurface(displayConfig->display, eglSurface)) {
            Log::Warning(Event::OpenGL, eglErrorMessage("eglDestroySurface"));
        }
        eglSurface = EGL_NO_SURFACE;
    }

    if (eglContext != EGL_NO_CONTEXT) {
        if (!eglDestroyContext(displayConfig->display, eglContext)) {
            Log::Warning(Event::OpenGL, eglErrorMessage("eglDestroyContext"));
        }
        eglContext = EGL_NO_CONTEXT;
    }

    if (window != nullptr) {
        OH_NativeWindow_NativeObjectUnreference(window);
        window = nullptr;
    }
}

void EGLWindowBackend::setSize(Size size_) {
    size = size_;
    if (!setNativeWindowBufferGeometry(window, size)) {
        Log::Warning(Event::OpenGL, "OH_NativeWindow_NativeWindowHandleOpt SET_BUFFER_GEOMETRY failed");
    }
}

void EGLWindowBackend::swap() {
    MLN_TRACE_FUNC();

    if (!eglSwapBuffers(displayConfig->display, eglSurface)) {
        Log::Warning(Event::OpenGL, eglErrorMessage("eglSwapBuffers"));
    }
}

void EGLWindowBackend::activate() {
    MLN_TRACE_FUNC();

    if (!eglMakeCurrent(displayConfig->display, eglSurface, eglSurface, eglContext)) {
        throw std::runtime_error(eglErrorMessage("eglMakeCurrent"));
    }
    if (framebufferDiagnostic.empty()) {
        framebufferDiagnostic = formatDefaultFramebuffer();
        logDiagnostic(framebufferDiagnostic);
    }
}

void EGLWindowBackend::deactivate() {
    MLN_TRACE_FUNC();

    if (!eglMakeCurrent(displayConfig->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        throw std::runtime_error(eglErrorMessage("eglMakeCurrent"));
    }
}

gl::ProcAddress EGLWindowBackend::getExtensionFunctionPointer(const char* name) {
    MLN_TRACE_FUNC();

    return eglGetProcAddress(name);
}

void EGLWindowBackend::updateAssumedState() {
    MLN_TRACE_FUNC();

    assumeFramebufferBinding(0);
    assumeViewport(0, 0, size);
}

} // namespace ohos
} // namespace mbgl
