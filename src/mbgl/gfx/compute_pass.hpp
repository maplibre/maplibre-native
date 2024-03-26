#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

namespace mbgl {
namespace gfx {

class Context;

class ComputePass {
public:
    ComputePass(gfx::Context& context);
    virtual ~ComputePass() = default;
    ComputePass(const ComputePass&) = delete;
    ComputePass& operator=(const ComputePass&) = delete;
    
    virtual void computeDrawableBuffer(std::vector<shaders::SymbolComputeUBO>& computeUBOVector,
                                       gfx::UniformBufferPtr& computeBuffer,
                                       gfx::UniformBufferPtr& drawableBuffer);
    
protected:
    gfx::Context& context;
};

} // namespace gfx
} // namespace mbgl
