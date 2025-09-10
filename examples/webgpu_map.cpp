// WebGPU Map Example - Renders MapLibre Native map using WebGPU backend
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/default_styles.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/timer.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/database_file_source.hpp>
#include <mbgl/storage/file_source_manager.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/gfx/backend.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/context.hpp>

#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include <dawn/native/MetalBackend.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#endif

#include <iostream>
#include <memory>
#include <csignal>

class WebGPUMapView : public mbgl::MapObserver {
public:
    WebGPUMapView() {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        
        // Create window without default API
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(1024, 768, "MapLibre Native - WebGPU", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }
        
        // Initialize WebGPU
        initWebGPU();
        
        // Create MapLibre components
        initMap();
    }
    
    ~WebGPUMapView() {
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
    
    void run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // Render frame
            render();
            
            // Process MapLibre events
            runLoop.runOnce();
        }
    }
    
    // MapObserver implementation
    void onCameraWillChange(mbgl::MapObserver::CameraChangeMode) override {}
    void onCameraIsChanging() override {}
    void onCameraDidChange(mbgl::MapObserver::CameraChangeMode) override {}
    void onWillStartLoadingMap() override {}
    void onDidFinishLoadingMap() override {
        mbgl::Log::Info(mbgl::Event::General, "Map finished loading");
    }
    void onDidFailLoadingMap(mbgl::MapLoadError, const std::string& message) override {
        mbgl::Log::Error(mbgl::Event::General, "Map failed to load: " + message);
    }
    void onWillStartRenderingFrame() override {}
    void onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus) override {}
    void onWillStartRenderingMap() override {}
    void onDidFinishRenderingMap(mbgl::MapObserver::RenderMode) override {}
    void onDidFinishLoadingStyle() override {
        mbgl::Log::Info(mbgl::Event::General, "Style finished loading");
    }
    void onSourceChanged(mbgl::style::Source&) override {}
    
private:
    void initWebGPU() {
        // Create Dawn instance
        instance = std::make_unique<dawn::native::Instance>();
        
        // Get adapters
        auto adapters = instance->EnumerateAdapters();
        if (adapters.empty()) {
            throw std::runtime_error("No WebGPU adapters found");
        }
        
        // Select first adapter
        adapter = adapters[0];
        
        // Create device
        wgpuDevice = adapter.CreateDevice();
        if (!wgpuDevice) {
            throw std::runtime_error("Failed to create WebGPU device");
        }
        
        device = wgpu::Device::Acquire(wgpuDevice);
        queue = device.GetQueue();
        
        // Setup surface
#ifdef __APPLE__
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        NSView* view = [nsWindow contentView];
        [view setWantsLayer:YES];
        
        metalLayer = [CAMetalLayer layer];
        metalLayer.device = dawn::native::metal::GetMTLDevice(wgpuDevice);
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        metalLayer.drawableSize = CGSizeMake(width, height);
        [view setLayer:metalLayer];
        
        // Create surface
        wgpu::SurfaceDescriptorFromMetalLayer metalDesc;
        metalDesc.layer = metalLayer;
        
        wgpu::SurfaceDescriptor surfaceDesc;
        surfaceDesc.nextInChain = &metalDesc;
        
        surface = wgpu::Surface::Acquire(instance->CreateSurface(&surfaceDesc).Release());
#endif
        
        // Configure swap chain
        wgpu::SwapChainDescriptor swapChainDesc = {};
        swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
        swapChainDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
        
        swapChain = device.CreateSwapChain(surface, &swapChainDesc);
        
        mbgl::Log::Info(mbgl::Event::General, "WebGPU initialized successfully");
    }
    
    void initMap() {
        // Create renderer backend
        backend = std::make_unique<mbgl::webgpu::RendererBackend>(
            mbgl::gfx::ContextMode::Unique);
        
        // Set the WebGPU device and surface
        backend->setDevice(wgpuDevice);
        backend->setSurface(surface.Get());
        
        // Create renderer
        float pixelRatio = getPixelRatio();
        renderer = std::make_unique<mbgl::Renderer>(*backend, pixelRatio);
        
        // Setup file source and resource options
        mbgl::ResourceOptions resourceOptions;
        resourceOptions.withCachePath("/tmp/mbgl-webgpu-cache.db");
        
        mbgl::ClientOptions clientOptions;
        
        // Create map
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        mbgl::MapOptions mapOptions;
        mapOptions.withSize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)})
                  .withPixelRatio(pixelRatio);
        
        map = std::make_unique<mbgl::Map>(*renderer, *this, mapOptions, resourceOptions, clientOptions);
        
        // Load a default style
        map->getStyle().loadURL("https://demotiles.maplibre.org/style.json");
        
        // Set initial position
        map->jumpTo(mbgl::CameraOptions()
            .withCenter(mbgl::LatLng{37.7749, -122.4194}) // San Francisco
            .withZoom(10.0));
        
        mbgl::Log::Info(mbgl::Event::General, "Map initialized");
    }
    
    void render() {
        if (!map || !renderer) return;
        
        // Get current swap chain texture
        wgpu::TextureView textureView = swapChain.GetCurrentTextureView();
        if (!textureView) {
            mbgl::Log::Warning(mbgl::Event::General, "Failed to get swap chain texture");
            return;
        }
        
        // Create render pass
        wgpu::RenderPassColorAttachment colorAttachment{};
        colorAttachment.view = textureView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.1f, 0.2f, 0.3f, 1.0f}; // Dark blue background
        
        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        
        // Render the map
        // Note: This is where the actual map rendering would happen
        // The WebGPU backend would handle all the tile rendering, text rendering, etc.
        renderer->render(map->renderSync());
        
        pass.End();
        
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        swapChain.Present();
    }
    
    float getPixelRatio() const {
        int fbWidth, fbHeight;
        int winWidth, winHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glfwGetWindowSize(window, &winWidth, &winHeight);
        return static_cast<float>(fbWidth) / static_cast<float>(winWidth);
    }
    
private:
    // GLFW
    GLFWwindow* window = nullptr;
    
    // WebGPU
    std::unique_ptr<dawn::native::Instance> instance;
    dawn::native::Adapter adapter;
    WGPUDevice wgpuDevice = nullptr;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    wgpu::SwapChain swapChain;
    
#ifdef __APPLE__
    CAMetalLayer* metalLayer = nullptr;
#endif
    
    // MapLibre
    mbgl::util::RunLoop runLoop;
    std::unique_ptr<mbgl::webgpu::RendererBackend> backend;
    std::unique_ptr<mbgl::Renderer> renderer;
    std::unique_ptr<mbgl::Map> map;
};

// Signal handler for clean shutdown
static WebGPUMapView* globalView = nullptr;

void signalHandler(int) {
    if (globalView) {
        mbgl::Log::Info(mbgl::Event::General, "Shutting down...");
        std::exit(0);
    }
}

int main(int argc, char* argv[]) {
    // Setup signal handler
    std::signal(SIGINT, signalHandler);
    
    try {
        mbgl::Log::Info(mbgl::Event::General, "Starting MapLibre Native with WebGPU backend");
        
        WebGPUMapView view;
        globalView = &view;
        
        view.run();
        
        globalView = nullptr;
        
    } catch (const std::exception& e) {
        mbgl::Log::Error(mbgl::Event::General, std::string("Error: ") + e.what());
        return 1;
    }
    
    return 0;
}