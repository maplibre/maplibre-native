// Minimal WebGPU Dawn example - just creates instance and device
#include <iostream>
#include <dawn/native/DawnNative.h>
#include <GLFW/glfw3.h>

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // Create window with no graphics API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "WebGPU Dawn Minimal", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    
    std::cout << "✓ Created GLFW window\n";
    
    // Create Dawn instance
    auto instance = std::make_unique<dawn::native::Instance>();
    std::cout << "✓ Created Dawn instance\n";
    
    // Get adapters
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters();
    std::cout << "Found " << adapters.size() << " adapter(s)\n";
    
    if (adapters.empty()) {
        std::cerr << "No adapters found\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // Use first adapter
    dawn::native::Adapter& adapter = adapters[0];
    std::cout << "✓ Selected adapter\n";
    
    // Create device
    WGPUDevice device = adapter.CreateDevice();
    if (!device) {
        std::cerr << "Failed to create device\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    std::cout << "✓ Created WebGPU device\n";
    std::cout << "\nWebGPU Dawn is working! Close the window to exit.\n";
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}