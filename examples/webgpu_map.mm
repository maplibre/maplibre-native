// WebGPU Map Example for MapLibre Native
// Renders a full MapLibre map using Dawn's Metal backend

#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>

// WebGPU backend
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>

// Dawn headers
#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <dawn/native/MetalBackend.h>
#include <webgpu/webgpu_cpp.h>

// GLFW headers
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Platform headers
#ifdef __APPLE__
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#endif

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>

namespace {
volatile bool running = true;
}

void signalHandler(int) {
    running = false;
}

namespace mbgl {
namespace webgpu {

// Custom WebGPU backend for MapLibre that integrates with Dawn
class MapRendererBackend : public RendererBackend {
public:
    MapRendererBackend(GLFWwindow* window_, wgpu::Device device_, wgpu::Surface surface_)
        : RendererBackend(gfx::ContextMode::Unique),
          window(window_),
          wgpuDevice(device_),
          wgpuSurface(surface_) {
        
        // Get window size
        glfwGetFramebufferSize(window, &width, &height);
        
        // Configure surface
        configureSurface();
        
        // Set the WebGPU device and surface in the base class
        setDevice(device_.Get());
        setSurface(surface_.Get());
        
        std::cout << "✓ MapRendererBackend initialized (" << width << "x" << height << ")\n";
    }
    
    ~MapRendererBackend() override = default;
    
    void updateAssumedState() override {
        // Update any WebGPU state assumptions
        // This is called when the context needs to be reset
    }
    
    gfx::Renderable& getDefaultRenderable() override {
        // Return the default renderable (the main surface)
        if (!defaultRenderable) {
            // Create a default renderable that points to the surface
            class SurfaceRenderable : public gfx::Renderable {
            public:
                SurfaceRenderable(Size size_) : size(size_) {}
                Size getSize() const override { return size; }
            private:
                Size size;
            };
            defaultRenderable = std::make_unique<SurfaceRenderable>(Size{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        }
        return *defaultRenderable;
    }
    
    void beginFrame() {
        // Get the current texture from the surface
        wgpuSurface.GetCurrentTexture(&currentSurfaceTexture);
        
        if (currentSurfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
            currentSurfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
            std::cerr << "Failed to get surface texture\n";
            return;
        }
        
        currentTextureView = currentSurfaceTexture.texture.CreateView();
    }
    
    void endFrame() {
        // Present the frame
        wgpuSurface.Present();
        currentTextureView = nullptr;
    }
    
    wgpu::TextureView getCurrentTextureView() const {
        return currentTextureView;
    }
    
    wgpu::Device getWGPUDevice() const {
        return wgpuDevice;
    }
    
    Size getFramebufferSize() const {
        return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }
    
    void onResize(int newWidth, int newHeight) {
        width = newWidth;
        height = newHeight;
        configureSurface();
        
        // Update the default renderable
        defaultRenderable.reset();
    }
    
private:
    void configureSurface() {
        wgpu::SurfaceConfiguration config = {};
        config.device = wgpuDevice;
        config.usage = wgpu::TextureUsage::RenderAttachment;
        config.format = wgpu::TextureFormat::BGRA8Unorm;
        config.width = width;
        config.height = height;
        config.presentMode = wgpu::PresentMode::Fifo;
        config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
        
        wgpuSurface.Configure(&config);
    }
    
    GLFWwindow* window;
    wgpu::Device wgpuDevice;
    wgpu::Surface wgpuSurface;
    wgpu::SurfaceTexture currentSurfaceTexture;
    wgpu::TextureView currentTextureView;
    int width, height;
    std::unique_ptr<gfx::Renderable> defaultRenderable;
};

} // namespace webgpu
} // namespace mbgl

// Map observer to track map events
class MapObserver : public mbgl::MapObserver {
public:
    void onWillStartLoadingMap() override {
        std::cout << "Map: Starting to load...\n";
    }
    
    void onDidFinishLoadingMap() override {
        std::cout << "Map: Finished loading!\n";
    }
    
    void onDidFailLoadingMap(mbgl::MapLoadError, const std::string& error) override {
        std::cerr << "Map: Failed to load - " << error << "\n";
    }
    
    void onDidFinishLoadingStyle() override {
        std::cout << "Map: Style loaded\n";
    }
    
    void onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus status) override {
        if (status.mode == mbgl::MapObserver::RenderMode::Full) {
            frameCount++;
            if (frameCount % 60 == 0) {
                std::cout << "Rendered " << frameCount << " frames\n";
            }
        }
    }
    
private:
    int frameCount = 0;
};

class MapApplication {
public:
    MapApplication() {}
    
    bool initialize(GLFWwindow* window) {
        this->window = window;
        
        std::cout << "Initializing WebGPU for MapLibre Map...\n";
        
        // Initialize Dawn
        dawnProcSetProcs(&dawn::native::GetProcs());
        
        // Get Metal adapter
        wgpu::RequestAdapterOptions adapterOptions = {};
        adapterOptions.backendType = wgpu::BackendType::Metal;
        
        std::vector<dawn::native::Adapter> adapters = dawnInstance.EnumerateAdapters(&adapterOptions);
        
        if (adapters.empty()) {
            std::cerr << "No Metal adapter found\n";
            return false;
        }
        
        adapter = wgpu::Adapter(adapters[0].Get());
        
        // Get adapter info
        wgpu::AdapterInfo info = {};
        adapter.GetInfo(&info);
        std::cout << "✓ Using Metal adapter";
        if (info.device.data) {
            std::cout << ": " << std::string(info.device.data, info.device.length);
        }
        std::cout << "\n";
        
        // Create surface
        if (!createSurface()) {
            return false;
        }
        
        // Create device
        if (!createDevice()) {
            return false;
        }
        
        std::cout << "✓ WebGPU initialized\n";
        
        // Initialize MapLibre
        if (!initializeMap()) {
            return false;
        }
        
        return true;
    }
    
    void render() {
        if (!map || !backend) return;
        
        // Begin frame
        backend->beginFrame();
        
        // Render the map
        try {
            map->render(*backend);
        } catch (const std::exception& e) {
            std::cerr << "Render error: " << e.what() << "\n";
        }
        
        // End frame (presents)
        backend->endFrame();
    }
    
    void cleanup() {
        map.reset();
        renderer.reset();
        backend.reset();
    }
    
    void onResize(int width, int height) {
        if (backend) {
            backend->onResize(width, height);
        }
        if (map) {
            map->setSize(backend->getFramebufferSize());
        }
    }
    
    void onMouseMove(double x, double y) {
        if (map && isDragging) {
            double dx = x - lastX;
            double dy = y - lastY;
            
            // Pan the map
            map->moveBy(mbgl::ScreenCoordinate{dx, dy});
            
            lastX = x;
            lastY = y;
        }
    }
    
    void onMouseButton(int button, int action, double x, double y) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                isDragging = true;
                lastX = x;
                lastY = y;
            } else if (action == GLFW_RELEASE) {
                isDragging = false;
            }
        }
    }
    
    void onScroll(double xOffset, double yOffset) {
        if (map) {
            // Get current mouse position for zoom center
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            
            // Zoom the map
            double zoomDelta = yOffset * 0.5;
            map->scaleBy(std::exp2(zoomDelta), mbgl::ScreenCoordinate{x, y});
        }
    }

private:
    bool createSurface() {
#ifdef __APPLE__
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        if (!nsWindow) {
            std::cerr << "Failed to get NSWindow from GLFW\n";
            return false;
        }
        
        // Create Metal layer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        [nsWindow.contentView setWantsLayer:YES];
        [nsWindow.contentView setLayer:metalLayer];
        
        // Get window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        [metalLayer setDrawableSize:CGSizeMake(width, height)];
        
        // Set pixel format
        [metalLayer setPixelFormat:MTLPixelFormatBGRA8Unorm];
        
        // Create surface descriptor
        wgpu::SurfaceDescriptorFromMetalLayer metalDesc = {};
        metalDesc.layer = (__bridge void*)metalLayer;
        
        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &metalDesc;
        surfaceDesc.label = "MapLibre Surface";
        
        // Create surface
        wgpu::Instance instance = wgpu::Instance(dawnInstance.Get());
        surface = instance.CreateSurface(&surfaceDesc);
        
        if (!surface) {
            std::cerr << "Failed to create surface\n";
            return false;
        }
        
        std::cout << "✓ Created Metal surface\n";
        return true;
#else
        std::cerr << "Only macOS is supported\n";
        return false;
#endif
    }
    
    bool createDevice() {
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "MapLibre Device";
        
        // Request features needed for MapLibre
        std::vector<wgpu::FeatureName> features;
        deviceDesc.requiredFeatures = features.data();
        deviceDesc.requiredFeatureCount = features.size();
        
        device = adapter.CreateDevice(&deviceDesc);
        if (!device) {
            std::cerr << "Failed to create device\n";
            return false;
        }
        
        // Set up error callback
        device.SetLoggingCallback(
            [](wgpu::LoggingType type, char const * message) {
                if (type == wgpu::LoggingType::Error) {
                    std::cerr << "WebGPU Error: " << message << "\n";
                }
            });
        
        queue = device.GetQueue();
        std::cout << "✓ Created device and queue\n";
        return true;
    }
    
    bool initializeMap() {
        // Create WebGPU backend
        backend = std::make_unique<mbgl::webgpu::MapRendererBackend>(window, device, surface);
        
        // Create renderer
        renderer = std::make_unique<mbgl::Renderer>(*backend, 1.0f);
        
        // Create map observer
        mapObserver = std::make_unique<MapObserver>();
        
        // Create map
        mbgl::MapOptions mapOptions;
        mapOptions.withMapMode(mbgl::MapMode::Continuous)
                 .withSize(backend->getFramebufferSize())
                 .withPixelRatio(2.0f);  // Retina display
        
        map = std::make_unique<mbgl::Map>(
            *renderer,
            mapObserver.get(),
            mapOptions,
            mbgl::ResourceOptions()
                .withCachePath(":memory:")
                .withAssetPath(".")
                .withMaximumCacheSize(64 * 1024 * 1024));
        
        // Load a style
        std::string styleUrl = "https://demotiles.maplibre.org/style.json";
        std::cout << "Loading map style from: " << styleUrl << "\n";
        map->getStyle().loadURL(styleUrl);
        
        // Set initial position (San Francisco)
        map->jumpTo(mbgl::CameraOptions()
            .withCenter(mbgl::LatLng(37.7749, -122.4194))
            .withZoom(10.0)
            .withPitch(0.0)
            .withBearing(0.0));
        
        std::cout << "✓ MapLibre map initialized\n";
        
        return true;
    }
    
private:
    GLFWwindow* window = nullptr;
    dawn::native::Instance dawnInstance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    
    std::unique_ptr<mbgl::webgpu::MapRendererBackend> backend;
    std::unique_ptr<mbgl::Renderer> renderer;
    std::unique_ptr<mbgl::Map> map;
    std::unique_ptr<MapObserver> mapObserver;
    mbgl::util::RunLoop runLoop;
    
    bool isDragging = false;
    double lastX = 0, lastY = 0;
};

// Global app instance for callbacks
MapApplication* g_app = nullptr;

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    if (g_app) {
        g_app->onResize(width, height);
    }
}

void cursorPosCallback(GLFWwindow*, double x, double y) {
    if (g_app) {
        g_app->onMouseMove(x, y);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (g_app) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        g_app->onMouseButton(button, action, x, y);
    }
}

void scrollCallback(GLFWwindow*, double xOffset, double yOffset) {
    if (g_app) {
        g_app->onScroll(xOffset, yOffset);
    }
}

int main() {
    std::cout << "MapLibre Native - WebGPU Map\n";
    std::cout << "=============================\n\n";
    
    // Set up signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }
    
    // Configure GLFW for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1024, 768, "MapLibre Native - WebGPU", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    
    std::cout << "✓ Created GLFW window (1024x768)\n";
    
    // Setup callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    // Initialize application
    MapApplication app;
    g_app = &app;
    
    if (!app.initialize(window)) {
        std::cerr << "Failed to initialize application\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    std::cout << "\n🗺️  MapLibre Native rendering with WebGPU!\n";
    std::cout << "Controls:\n";
    std::cout << "  • Click and drag to pan\n";
    std::cout << "  • Scroll to zoom\n";
    std::cout << "  • ESC to exit\n\n";
    
    // Main loop
    while (!glfwWindowShouldClose(window) && running) {
        glfwPollEvents();
        
        // Check for ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        // Render
        app.render();
        
        // Small delay to control frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    // Cleanup
    std::cout << "\nShutting down...\n";
    app.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "✓ Cleanup complete\n";
    return 0;
}