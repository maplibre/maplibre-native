#pragma once

#include <mbgl/webgpu/backend_impl.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <unordered_map>
#include <string>

namespace mbgl {
namespace webgpu {

class Context::Impl {
public:
    Impl() = default;
    ~Impl() = default;
    
    WGPUDevice getDevice() const { return reinterpret_cast<WGPUDevice>(device); }
    WGPUQueue getQueue() const { return reinterpret_cast<WGPUQueue>(queue); }
    
    void* device = nullptr;
    void* queue = nullptr;
    std::unordered_map<std::string, gfx::ShaderProgramBasePtr> shaderCache;
};

} // namespace webgpu
} // namespace mbgl