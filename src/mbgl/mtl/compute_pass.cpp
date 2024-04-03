#include <mbgl/mtl/compute_pass.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace mtl {

using namespace shaders;

static constexpr auto computeSource = R"(
#include <metal_stdlib>
using namespace metal;

struct alignas(16) SymbolComputeUBO {
    float4x4 projMatrix;
    
    int32_t layerIndex;
    int32_t subLayerIndex;
    uint32_t width;
    uint32_t height;

    float2 texsize;
    float2 texsize_icon;

    uint32_t tileIdCanonicalX;
    uint32_t tileIdCanonicalY;
    uint16_t tileIdCanonicalZ;
    int16_t tileIdWrap;
    float camDist;

    float scale;
    float bearing;
    float zoom;
    float pitch;
    
    float2 translation;
    
    bool isAnchorMap;
    bool inViewportPixelUnits;
    bool pitchWithMap;
    bool rotateWithMap;
    bool alongLine;
    bool hasVariablePlacement;

    int16_t padding;
};
static_assert(sizeof(SymbolComputeUBO) == 9 * 16, "unexpected padding");

struct alignas(16) SymbolDrawableUBO {
    float4x4 matrix;
    float4x4 label_plane_matrix;
    float4x4 coord_matrix;

    float2 texsize;
    float2 texsize_icon;

    float gamma_scale;
    /*bool*/ int rotate_symbol;
    float2 pad;
};
static_assert(sizeof(SymbolDrawableUBO) == 14 * 16, "unexpected padding");

namespace util {
constant constexpr int32_t EXTENT = 8192;
constant constexpr float tileSize_D = 512;

float2 rotate(float2 a, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float x = cosA * a.x - sinA * a.y;
    float y = sinA * a.x + cosA * a.y;
    return float2(x, y);
}
}

namespace PaintParameters {
constant constexpr int numSublayers = 3;
constant constexpr float depthEpsilon = 1.0 / (1 << 12);
}

namespace Projection {
float worldSize(float scale) {
    return scale * util::tileSize_D;
}
}

namespace matrix2 {

float4x4 translate(float4x4 a, float x, float y, float z) {
    return a * float4x4(1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        x, y, z, 1);
}

float4x4 scale(float4x4 a, float x, float y, float z) {
    return a * float4x4(x, 0, 0, 0,
                        0, y, 0, 0,
                        0, 0, z, 0,
                        0, 0, 0, 1);
}

float4x4 multiply(float4x4 a, float4x4 b) {
    return a * b;
}

float4x4 rotate_z(float4x4 a, float rad) {
    float s = sin(rad);
    float c = cos(rad);
    return a * float4x4(c,  c, 0, 0,
                        s, -s, 0, 0,
                        0,  0, 1, 0,
                        0,  0, 0, 1);
}

}

float pixelsToTileUnits(const float pixelValue, const uint16_t tileIdCanonicalZ, const float zoom) {
    return pixelValue * (util::EXTENT / (util::tileSize_D * pow(2.f, zoom - tileIdCanonicalZ)));
}

float4x4 computeTileMatrix(device const SymbolComputeUBO& computeUBO) {
    // from RenderTile::prepare
    const uint64_t tileScale = 1ull << computeUBO.tileIdCanonicalZ;
    const float s = Projection::worldSize(computeUBO.scale) / tileScale;
    
    float4x4 tileMatrix(1.0);
    tileMatrix = matrix2::translate(tileMatrix,
                                    int64_t(computeUBO.tileIdCanonicalX + computeUBO.tileIdWrap * static_cast<int64_t>(tileScale)) * s,
                                    int64_t(computeUBO.tileIdCanonicalY) * s,
                                    0);
    tileMatrix = matrix2::scale(tileMatrix, s / util::EXTENT, s / util::EXTENT, 1);
    
    computeUBO.projMatrix[3][2] -= ((1 + computeUBO.layerIndex) * PaintParameters::numSublayers - computeUBO.subLayerIndex) * PaintParameters::depthEpsilon;
    tileMatrix = matrix2::multiply(computeUBO.projMatrix, tileMatrix);
    
    if (computeUBO.translation[0] == 0 && computeUBO.translation[1] == 0) {
        return tileMatrix;
    }
    
    float4x4 vtxMatrix;
    
    const float angle = computeUBO.inViewportPixelUnits
                            ? (computeUBO.isAnchorMap ? computeUBO.bearing : 0.0f)
                            : (computeUBO.isAnchorMap ? 0.0f : -computeUBO.bearing);
    
    float2 translate = util::rotate(computeUBO.translation, angle);
    
    if (computeUBO.inViewportPixelUnits) {
        vtxMatrix = matrix2::translate(tileMatrix, translate.x, translate.y, 0);
    } else {
        vtxMatrix = matrix2::translate(tileMatrix,
                                       pixelsToTileUnits(translate.x, computeUBO.tileIdCanonicalZ, computeUBO.zoom),
                                       pixelsToTileUnits(translate.y, computeUBO.tileIdCanonicalZ, computeUBO.zoom),
                                       0);
    }
    
    return vtxMatrix;
}

float4x4 computeLabelPlaneMatrix(const float4x4 posMatrix,
                                 device const SymbolComputeUBO& computeUBO,
                                 const float pixelsToTileUnits) {
    if (computeUBO.alongLine || computeUBO.hasVariablePlacement) {
        return float4x4(1.0);
    }
    
    float4x4 m(1.0);
    if (computeUBO.pitchWithMap) {
        m = matrix2::scale(m, 1 / pixelsToTileUnits, 1 / pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            m = matrix2::rotate_z(m, computeUBO.bearing);
        }
    } else {
        m = matrix2::scale(m, computeUBO.width / 2.0, -(computeUBO.height / 2.0), 1.0);
        m = matrix2::translate(m, 1, -1, 0);
        m = matrix2::multiply(m, posMatrix);
    }
    return m;
}

float4x4 computeGlCoordMatrix(const float4x4 posMatrix,
                              device const SymbolComputeUBO& computeUBO,
                              const float pixelsToTileUnits) {
    float4x4 m(1.0);
    if (computeUBO.pitchWithMap) {
        m = matrix2::multiply(m, posMatrix);
        m = matrix2::scale(m, pixelsToTileUnits, pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            m = matrix2::rotate_z(m, -computeUBO.bearing);
        }
    } else {
        m = matrix2::scale(m, 1, -1, 1);
        m = matrix2::translate(m, -1, -1, 0);
        m = matrix2::scale(m, 2.0 / computeUBO.width, 2.0 / computeUBO.height, 1.0);
    }
    return m;
}

kernel void kernelMain(device const SymbolComputeUBO* computeUBOVector [[buffer(0)]],
                       device SymbolDrawableUBO* drawableUBOVector [[buffer(1)]],
                       constant uint& size [[buffer(2)]],
                       uint index [[thread_position_in_grid]]) {
    if (index >= size) {
        return;
    }
    device const SymbolComputeUBO& computeUBO = computeUBOVector[index];
    
    const auto matrix = computeTileMatrix(computeUBO);
    
    // from symbol_program, makeValues
    const auto currentZoom = computeUBO.zoom;
    const float pixelsToUnits = pixelsToTileUnits(1.f, computeUBO.tileIdCanonicalZ, currentZoom);
    
    const float4x4 labelPlaneMatrix = computeLabelPlaneMatrix(matrix, computeUBO, pixelsToUnits);
    const float4x4 glCoordMatrix = computeGlCoordMatrix(matrix, computeUBO, pixelsToUnits);
    
    const float gammaScale = (computeUBO.pitchWithMap ? cos(computeUBO.pitch) * computeUBO.camDist : 1.0f);
    
    // Line label rotation happens in `updateLineLabels`/`reprojectLineLabels``
    // Pitched point labels are automatically rotated by the labelPlaneMatrix projection
    // Unpitched point labels need to have their rotation applied after projection
    const bool rotateInShader = computeUBO.rotateWithMap && !computeUBO.pitchWithMap && !computeUBO.alongLine;
    
    drawableUBOVector[index] = {
        matrix,
        labelPlaneMatrix,
        glCoordMatrix,
        
        computeUBO.texsize,
        computeUBO.texsize_icon,
        
        gammaScale,
        rotateInShader,
        {0},
    };
}

)";

MTLFunctionPtr createComputeFunction(const std::string_view source,
                                     const std::string_view kernelName,
                                     const MTLDevicePtr& device) {
    const auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    auto options = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    options->setFastMathEnabled(true);
    options->setLanguageVersion(MTL::LanguageVersion2_1);

    // TODO: Compile common code into a `LibraryTypeDynamic` to be used by other shaders
    // instead of duplicating that code in each and every shader compilation.
    options->setLibraryType(MTL::LibraryTypeExecutable);

    // Allows use of the [[invariant]] attribute on position outputs to
    // guarantee that the GPU performs the calculations the same way.
    options->setPreserveInvariance(true);

    // TODO: Allow use of `LibraryOptimizationLevelSize` which "may also reduce compile time"
    // requires a check for iOS 16+
    // options->setOptimizationLevel(MTL::LibraryOptimizationLevelDefault);

    NS::Error* error = nullptr;
    NS::String* nsSource = NS::String::string(source.data(), NS::UTF8StringEncoding);

    MTLFunctionPtr kernelFunction;
    MTL::Library* library = device->newLibrary(nsSource, options.get(), &error);
    if (!library || error) {
        const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
        const auto errStr = (errPtr && errPtr[0]) ? ": " + std::string(errPtr) : std::string();
        Log::Error(Event::Shader, "compute shader compile failed" + errStr);
        assert(false);
        return kernelFunction;
    }

    const auto nsKernelName = NS::String::string(kernelName.data(), NS::UTF8StringEncoding);
    kernelFunction = NS::RetainPtr(library->newFunction(nsKernelName));
    if (!kernelFunction) {
        const auto kernelStr = (kernelName.data() && kernelName.data()[0]) ? ": " + std::string(kernelName.data()) : std::string();
        Log::Error(Event::Shader, "compute shader missing kernel function " + kernelStr);
        assert(false);
        return kernelFunction;
    }

    return kernelFunction;
}

MTLComputePipelineStatePtr computePipelineState;

ComputePass::ComputePass(gfx::Context& context)
    : gfx::ComputePass(context) {
    NS::Error* error = nullptr;
    const auto& mtlContext = static_cast<mtl::Context&>(context);
    const auto& device = mtlContext.getBackend().getDevice();
    if (!computePipelineState) {
        const auto computeFunction = createComputeFunction(computeSource, "kernelMain", device);
        computePipelineState = NS::TransferPtr(device->newComputePipelineState(computeFunction.get(), &error));
    }
    commandQueue = NS::TransferPtr(device->newCommandQueue());
    commandBuffer = NS::RetainPtr(commandQueue->commandBuffer());
}

ComputePass::~ComputePass() {
    if (computeCommandEncoder) {
        computeCommandEncoder->endEncoding();
        computeCommandEncoder.reset();
    }
    commandBuffer->commit();
    //commandBuffer->waitUntilCompleted();
}

void ComputePass::computeDrawableBuffer(std::vector<SymbolComputeUBO>& computeUBOVector, 
                                        gfx::UniformBufferPtr& computeBuffer,
                                        gfx::UniformBufferPtr& drawableBuffer) {
    
    if( !computeCommandEncoder ) {
        computeCommandEncoder = NS::RetainPtr(commandBuffer->computeCommandEncoder());
    }
    
    const size_t computeUBOVectorSize = sizeof(SymbolComputeUBO) * computeUBOVector.size();
    if (!computeBuffer || computeBuffer->getSize() < computeUBOVectorSize) {
        computeBuffer = context.createUniformBuffer(computeUBOVector.data(), computeUBOVectorSize, true);
    } else {
        computeBuffer->update(computeUBOVector.data(), computeUBOVectorSize);
    }
    
    const size_t drawableUBOVectorSize = sizeof(SymbolDrawableUBO) * computeUBOVector.size();
    if (!drawableBuffer || drawableBuffer->getSize() < drawableUBOVectorSize) {
        drawableBuffer = context.createUniformBuffer(nullptr, drawableUBOVectorSize, true);
    }
    
    const auto& mtlComputeBuffer = static_cast<const UniformBuffer&>(*computeBuffer);
    const auto& mtlDrawableBuffer = static_cast<const UniformBuffer&>(*drawableBuffer);
    const auto size = computeUBOVector.size();
    
    computeCommandEncoder->setComputePipelineState(computePipelineState.get());
    computeCommandEncoder->setBuffer(mtlComputeBuffer.getBufferResource().getMetalBuffer().get(), 0, 0);
    computeCommandEncoder->setBuffer(mtlDrawableBuffer.getBufferResource().getMetalBuffer().get(), 0, 1);
    computeCommandEncoder->setBytes(&size, sizeof(size), 2);
    
    MTL::Size gridSize = MTL::Size(computeUBOVector.size(), 1, 1);

    // Calculate a threadgroup size.
    NS::UInteger threadGroupSize = computePipelineState->maxTotalThreadsPerThreadgroup();
    if (threadGroupSize > computeUBOVector.size())
    {
        threadGroupSize = computeUBOVector.size();
    }
    MTL::Size threadgroupSize = MTL::Size(threadGroupSize, 1, 1);
    //computeCommandEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeCommandEncoder->dispatchThreadgroups(gridSize, threadgroupSize);
}

} // namespace gfx
} // namespace mbgl
