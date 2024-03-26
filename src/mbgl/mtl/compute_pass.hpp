#pragma once

#include <mbgl/gfx/compute_pass.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

namespace mbgl {
namespace mtl {

class ComputePass : public gfx::ComputePass {
public:
    ComputePass(gfx::Context& context);
    ~ComputePass() override;
    ComputePass(const ComputePass&) = delete;
    ComputePass& operator=(const ComputePass&) = delete;
    
    void computeDrawableBuffer(std::vector<shaders::SymbolComputeUBO>& computeUBOVector,
                               gfx::UniformBufferPtr& computeBuffer,
                               gfx::UniformBufferPtr& drawableBuffer) override;
    
private:
    MTLCommandQueuePtr commandQueue;
    MTLCommandBufferPtr commandBuffer;
    MTLComputeCommandEncoderPtr computeCommandEncoder;
};

} // namespace gfx
} // namespace mbgl
