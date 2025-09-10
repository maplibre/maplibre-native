#pragma once

#include <memory>

// Forward declarations for WebGPU types
// These will be properly defined based on the implementation (Dawn or wgpu)
struct WGPUDeviceImpl;
struct WGPUQueueImpl;
struct WGPUSwapChainImpl;
struct WGPUSurfaceImpl;
struct WGPUAdapterImpl;
struct WGPUInstanceImpl;

typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPUSwapChainImpl* WGPUSwapChain;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUInstanceImpl* WGPUInstance;

namespace mbgl {
namespace webgpu {

// Abstract interface for WebGPU backend implementation
// This allows us to support both Dawn and wgpu without tying the code to either
class BackendImpl {
public:
    virtual ~BackendImpl() = default;

    virtual WGPUInstance getInstance() const = 0;
    virtual WGPUAdapter getAdapter() const = 0;
    virtual WGPUDevice getDevice() const = 0;
    virtual WGPUQueue getQueue() const = 0;
    virtual WGPUSurface getSurface() const = 0;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    // Factory method to create the appropriate implementation
    static std::unique_ptr<BackendImpl> create();
};

} // namespace webgpu
} // namespace mbgl