#pragma once

#include <mbgl/util/logging.hpp>

#include <cassert>

#include <mbgl/gl/headless_backend.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#include "gl_functions_wgl.h"

namespace mbgl {
namespace gl {

// This class provides a singleton that contains information about the configuration used for
// instantiating new headless rendering contexts.
class WGLDisplayConfig {
private:
    // Key for singleton construction.
    struct Key { explicit Key() = default; };

    WNDCLASSEXA helperWindowClass = { sizeof(WNDCLASSEXA) };
    HWND helperWindowHandle = NULL;
    HDC helperWindowDeviceContext = NULL;
    HGLRC helperWindowRenderingContext = NULL;

private:
    void CreateHelperWindow() {
        MSG message;

        helperWindowClass.style = CS_OWNDC;
        helperWindowClass.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hWnd, uMsg, wParam, lParam); };
        helperWindowClass.lpszClassName = "WGL Helper Window";
        GetModuleHandleExA(0, NULL, &helperWindowClass.hInstance);

        if (!RegisterClassExA(&helperWindowClass)) {
            Log::Error(Event::OpenGL, "Failed to register helper window class");
            throw std::runtime_error("Failed to register helper window class");
        }

        helperWindowHandle = CreateWindowExA(
            0,
            helperWindowClass.lpszClassName,
            "WGL Helper Window",
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 1, 1,
            NULL,
            NULL,
            helperWindowClass.hInstance,
            NULL
        );

        if (!helperWindowHandle) {
            Log::Error(Event::OpenGL, "Failed to create helper window");
            throw std::runtime_error("Failed to create helper window");
        }

        ShowWindow(helperWindowHandle, SW_HIDE);

        while (PeekMessage(&message, helperWindowHandle, 0, 0, PM_REMOVE)) {
            DispatchMessage(&message);
        }
    }

    void DestroyHelperWindow() {
        if (helperWindowDeviceContext) {
            ReleaseDC(helperWindowHandle, helperWindowDeviceContext);
            DeleteDC(helperWindowDeviceContext);
        }

        DestroyWindow(helperWindowHandle);
        UnregisterClassA(helperWindowClass.lpszClassName, helperWindowClass.hInstance);
    }

    bool StringInExtensionString(const char* string, const char* extensions) {
        const char* start = extensions;

        while(true) {
            const char* where;
            const char* terminator;

            where = strstr(start, string);
            if (!where)
                return false;

            terminator = where + strlen(string);
            if (where == start || *(where - 1) == ' ') {
                if (*terminator == ' ' || *terminator == '\0')
                    break;
            }

            start = terminator;
        }

        return true;
    }

    bool extensionSupportedWGL(const char* extension) {
        const char* extensions = NULL;

        extensions = mbgl::platform::wglGetExtensionsStringARB(wglGetCurrentDC());

        if(!extensions || !StringInExtensionString(extension, extensions))
            extensions = mbgl::platform::wglGetExtensionsStringEXT();

        if(!extensions || !StringInExtensionString(extension, extensions))
            extensions = (const char *)wgl_glGetString(GL_EXTENSIONS);

        if (!extensions)
            return false;

        return StringInExtensionString(extension, extensions);
    }
public:
    explicit WGLDisplayConfig(Key) {
        HDC dummyDC, previousDC;
        HGLRC dummyRC, previousRC;
        PIXELFORMATDESCRIPTOR pfd;
        
        CreateHelperWindow();

        dummyDC = GetDC(helperWindowHandle);

        ZeroMemory(&pfd, sizeof(pfd));

        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_GENERIC_ACCELERATED | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        // Note: we're choosing an arbitrary pixel format, since we're not using the default surface
        // anyway; all rendering will be directed to framebuffers which have their own configuration.
        if (!SetPixelFormat(dummyDC, ChoosePixelFormat(dummyDC, &pfd), &pfd)) {
            Log::Error(Event::OpenGL, "Failed to set pixel format for dummy context");
            throw std::runtime_error("Failed to set pixel format for dummy context");
        }

        dummyRC = wglCreateContext(dummyDC);

        if (!dummyRC) {
            Log::Error(Event::OpenGL, "Failed to create dummy context");
            throw std::runtime_error("Failed to create dummy context");
        }

        previousDC = wglGetCurrentDC();
        previousRC = wglGetCurrentContext();

        if (!wglMakeCurrent(dummyDC, dummyRC)) {
            wglMakeCurrent(previousDC, previousRC);
            wglDeleteContext(dummyRC);

            DestroyHelperWindow();

            Log::Error(Event::OpenGL, "Failed to make dummy context current");
            throw std::runtime_error("Failed to make dummy context current");
        }

        ARB_multisample = extensionSupportedWGL("WGL_ARB_multisample");
        ARB_create_context = extensionSupportedWGL("WGL_ARB_create_context");
        ARB_create_context_profile = extensionSupportedWGL("WGL_ARB_create_context_profile");
        EXT_create_context_es2_profile = extensionSupportedWGL("WGL_EXT_create_context_es2_profile");
        EXT_colorspace = extensionSupportedWGL("WGL_EXT_colorspace");
        ARB_pixel_format = extensionSupportedWGL("WGL_ARB_pixel_format");
        ARB_ES2_compatibility = extensionSupportedWGL("GL_ARB_ES2_compatibility");

        if (!ARB_create_context || !ARB_create_context_profile || !(EXT_create_context_es2_profile || ARB_ES2_compatibility)) {
            Log::Error(Event::OpenGL, "OpenGL ES 2.0 is unavailable");
            throw std::runtime_error("OpenGL ES 2.0 is unavailable");
        }

        DestroyHelperWindow();

        renderingWindowClass.style = CS_OWNDC;
        renderingWindowClass.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hWnd, uMsg, wParam, lParam); };
        renderingWindowClass.lpszClassName = "WGL Rendering Window";
        GetModuleHandleExA(0, NULL, &renderingWindowClass.hInstance);

        if (!RegisterClassExA(&renderingWindowClass)) {
            Log::Error(Event::OpenGL, "Failed to register rendering window class");
            throw std::runtime_error("Failed to register rendering window class");
        }
    }

    ~WGLDisplayConfig() {
        UnregisterClassA(renderingWindowClass.lpszClassName, renderingWindowClass.hInstance);
    }

    static std::shared_ptr<WGLDisplayConfig> create() {
        static std::weak_ptr<WGLDisplayConfig> instance;
        auto shared = instance.lock();
        
        if (!shared) {
            instance = shared = std::make_shared<WGLDisplayConfig>(Key{});
        }
        
        return shared;
    }
public:
    bool ARB_multisample = false;
    bool ARB_create_context = false;
    bool ARB_create_context_profile = false;
    bool EXT_create_context_es2_profile = false;
    bool EXT_colorspace = false;
    bool ARB_pixel_format = false;
    bool ARB_ES2_compatibility = false;
    WNDCLASSEXA renderingWindowClass = { sizeof(WNDCLASSEXA) };
};

class WGLBackendImpl final : public HeadlessBackend::Impl {
private:
    std::shared_ptr<WGLDisplayConfig> wglDisplayConfig = WGLDisplayConfig::create();

    HWND renderingWindowHandle = NULL;
    HDC renderingWindowDeviceContext = NULL;
    HGLRC renderingWindowRenderingContext = NULL;
private:
    void CreateRenderingWindow() {
        MSG message;

        renderingWindowHandle = CreateWindowExA(
            0,
            wglDisplayConfig->renderingWindowClass.lpszClassName,
            "WGL Render Window",
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 1, 1,
            NULL,
            NULL,
            wglDisplayConfig->renderingWindowClass.hInstance,
            NULL
        );

        if (!renderingWindowHandle) {
            Log::Error(Event::OpenGL, "Failed to create helper window");
            throw std::runtime_error("Failed to create helper window");
        }

        ShowWindow(renderingWindowHandle, SW_HIDE);

        while (PeekMessage(&message, renderingWindowHandle, 0, 0, PM_REMOVE)) {
            DispatchMessage(&message);
        }
    }

    void DestroyRenderingWindow() {
        if (renderingWindowDeviceContext && renderingWindowHandle) {
            ReleaseDC(renderingWindowHandle, renderingWindowDeviceContext);
            DeleteDC(renderingWindowDeviceContext);
            renderingWindowDeviceContext = NULL;
        }

        if (renderingWindowHandle) {
            DestroyWindow(renderingWindowHandle);
            renderingWindowHandle = NULL;
        }
    }

    HGLRC GetContext() {
        if (!(renderingWindowRenderingContext || CreateRenderingContext())) {
            Log::Error(Event::OpenGL, "Failed to create rendering context");
            throw std::runtime_error("Failed to create rendering context");
        }

        return renderingWindowRenderingContext;
    }

    bool CreateRenderingContext() {
        PIXELFORMATDESCRIPTOR pfd;
        int pixelFormat;
        UINT numFormats;

        if (!renderingWindowHandle) {
            CreateRenderingWindow();
        }

        if (!renderingWindowDeviceContext) {
            renderingWindowDeviceContext = GetDC(renderingWindowHandle);
        }

        if (!mbgl::platform::wglChoosePixelFormatARB(renderingWindowDeviceContext, std::initializer_list<GLint>({
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 24,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            NULL
            }).begin(), NULL, 1, &pixelFormat, &numFormats)) {
            Log::Error(Event::OpenGL, "Failed to choose pixel format for context");
            throw std::runtime_error("Failed to choose pixel format for context");
        }

        ZeroMemory(&pfd, sizeof(pfd));

        pfd.nSize = sizeof(pfd);

        if (!DescribePixelFormat(renderingWindowDeviceContext, pixelFormat, sizeof(pfd), &pfd)) {
            Log::Error(Event::OpenGL, "Failed to retrieve pixel format for context");
            throw std::runtime_error("Failed to retrieve pixel format for context");
        }

        if (!SetPixelFormat(renderingWindowDeviceContext, pixelFormat, &pfd)) {
            Log::Error(Event::OpenGL, "Failed to set pixel format for context");
            throw std::runtime_error("Failed to set pixel format for context");
        }

        if (wglDisplayConfig->ARB_create_context) {
            renderingWindowRenderingContext = mbgl::platform::wglCreateContextAttribsARB(renderingWindowDeviceContext, NULL,
                std::initializer_list<int>({
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    NULL
                    }).begin());

            if (!renderingWindowRenderingContext) {
                const DWORD error = GetLastError();

                if (error == (0xc0070000 | ERROR_INVALID_VERSION_ARB)) {
                    Log::Error(Event::OpenGL, "Driver does not support OpenGL ES version 2.0");
                    throw std::runtime_error("Driver does not support OpenGL ES version 2.0");
                }
                else if (error == (0xc0070000 | ERROR_INVALID_PROFILE_ARB)) {
                    Log::Error(Event::OpenGL, "Driver does not support the requested OpenGL profile");
                    throw std::runtime_error("Driver does not support the requested OpenGL profile");
                }
                else if (error == (0xc0070000 | ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB)) {
                    Log::Error(Event::OpenGL, "The share context is not compatible with the requested context");
                    throw std::runtime_error("The share context is not compatible with the requested context");
                }
                else {
                    Log::Error(Event::OpenGL, "Failed to create OpenGL ES context");
                    throw std::runtime_error("Failed to create OpenGL ES context");
                }

                return false;
            }
        }
        else {
            renderingWindowRenderingContext = wglCreateContext(renderingWindowDeviceContext);

            if (!renderingWindowRenderingContext) {
                Log::Error(Event::OpenGL, "Failed to create OpenGL context");
                throw std::runtime_error("Failed to create OpenGL context");
            }
        }

        HGLRC currentRenderingContext = wglGetCurrentContext();

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(currentRenderingContext);

        wglMakeCurrent(renderingWindowDeviceContext, renderingWindowRenderingContext);

        return true;
    }

    bool DestroyRenderingContext() {
        if (renderingWindowRenderingContext && !wglMakeCurrent(renderingWindowDeviceContext, NULL)) {
            Log::Error(Event::OpenGL, "Failed to make context current");
            //throw std::runtime_error("Failed to make context current");
            return false;
        }

        if (renderingWindowRenderingContext && !wglDeleteContext(renderingWindowRenderingContext)) {
            Log::Error(Event::OpenGL, "Failed to delete current context");
            //throw std::runtime_error("Failed to delete current context");
            return false;
        }

        renderingWindowRenderingContext = NULL;

        return true;
    }

public:
    WGLBackendImpl() {
        auto renderingContext = GetContext();
    }

    ~WGLBackendImpl() final {
        if (renderingWindowRenderingContext) {
            DestroyRenderingContext();
        }

        DestroyRenderingWindow();
    }

    gl::ProcAddress getExtensionFunctionPointer(const char* name) final {
        return (ProcAddress)::wgl_GetProcAddress(name);
    }

    void activateContext() final {
        if (renderingWindowRenderingContext && !wglMakeCurrent(renderingWindowDeviceContext, renderingWindowRenderingContext)) {
            Log::Error(Event::OpenGL, "Switching OpenGL context failed");
            //throw std::runtime_error("Switching OpenGL context failed");
        }
    }

    void deactivateContext() final {
        if (renderingWindowRenderingContext && !wglMakeCurrent(NULL, NULL)) {
            Log::Error(Event::OpenGL, "Removing OpenGL context failed");
            //throw std::runtime_error("Removing OpenGL context failed");
        }
    }
};

void HeadlessBackend::createImpl() {
    assert(!impl);
    impl = std::make_unique<WGLBackendImpl>();
}

} // namespace gl
} // namespace mbgl
