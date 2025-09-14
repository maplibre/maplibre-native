#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/render_pass.hpp>
#include <mbgl/webgpu/renderer_backend.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/logging.hpp>
#include <memory>

namespace mbgl {
namespace webgpu {

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {
    // Uniform buffers are initialized in Impl constructor
}

Drawable::~Drawable() {
    // Clean up WebGPU resources safely
    // Note: We don't own the pipeline - it's owned by the shader program
    // So we should NOT release it here
    impl->pipeline = nullptr;
    
    if (impl->bindGroup) {
        wgpuBindGroupRelease(impl->bindGroup);
        impl->bindGroup = nullptr;
    }
    
    if (impl->uniformBuffer) {
        wgpuBufferRelease(impl->uniformBuffer);
        impl->uniformBuffer = nullptr;
    }
    
    if (impl->vertexBuffer) {
        wgpuBufferRelease(impl->vertexBuffer);
        impl->vertexBuffer = nullptr;
    }
    
    if (impl->indexBuffer) {
        wgpuBufferRelease(impl->indexBuffer);
        impl->indexBuffer = nullptr;
    }
    
    // Clear the bind group layout reference (we don't own it)
    impl->bindGroupLayout = nullptr;
}

void Drawable::upload(gfx::UploadPass& uploadPass) {

    auto& webgpuUploadPass = static_cast<webgpu::UploadPass&>(uploadPass);
    auto& gfxContext = webgpuUploadPass.getContext();
    auto& context = static_cast<webgpu::Context&>(gfxContext);
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) {
        return;
    }

    // Create bind group if we have a stored layout
    if (impl->bindGroupLayout && !impl->bindGroup) {

        // Create a uniform buffer for the transformation matrix if not exists
        if (!impl->uniformBuffer) {
            // Create identity matrix as placeholder - will be updated in draw()
            float matrix[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };

            WGPUBufferDescriptor uniformBufferDesc = {};
            WGPUStringView uniformLabel = {"Uniform Buffer", strlen("Uniform Buffer")};
            uniformBufferDesc.label = uniformLabel;
            uniformBufferDesc.size = sizeof(matrix);
            uniformBufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
            uniformBufferDesc.mappedAtCreation = 1;

            impl->uniformBuffer = wgpuDeviceCreateBuffer(device, &uniformBufferDesc);

            if (impl->uniformBuffer) {
                void* mappedData = wgpuBufferGetMappedRange(impl->uniformBuffer, 0, sizeof(matrix));
                if (mappedData) {
                    std::memcpy(mappedData, matrix, sizeof(matrix));
                    wgpuBufferUnmap(impl->uniformBuffer);
                }
            }
        }

        // Create bind group with uniform buffer
        WGPUBindGroupEntry uniformEntry = {};
        uniformEntry.binding = 0;
        uniformEntry.buffer = impl->uniformBuffer;
        uniformEntry.offset = 0;
        uniformEntry.size = 64; // 4x4 matrix

        WGPUBindGroupDescriptor bindGroupDesc = {};
        WGPUStringView label = {"Drawable Bind Group", strlen("Drawable Bind Group")};
        bindGroupDesc.label = label;
        bindGroupDesc.layout = impl->bindGroupLayout;
        bindGroupDesc.entryCount = 1;
        bindGroupDesc.entries = &uniformEntry;

        impl->bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
        if (impl->bindGroup) {
        } else {
        }
    }

    // Extract vertex data from attributes if needed
    if (impl->needsVertexExtraction && vertexAttributes) {

        // Look for the vertex attribute with our ID
        // For fill layers, the attribute might have index -1 but still contain the vertex data
        const auto& attrPtr = vertexAttributes->get(impl->vertexAttrId);
        if (attrPtr) {
            const auto& attr = *attrPtr;

            // Check if this attribute has shared raw data
            if (attr.getSharedRawData()) {
                    auto sharedData = attr.getSharedRawData();
                    auto offset = attr.getSharedOffset();
                    auto vertexOffset = attr.getSharedVertexOffset();
                    auto stride = attr.getSharedStride();
                    auto attrType = attr.getSharedType();


                    // Get the raw bytes from the shared data
                    const void* rawData = sharedData->getRawData();
                    std::size_t rawSize = sharedData->getRawSize();

                    // Calculate total size needed
                    std::size_t totalVertices = impl->vertexCount;
                    // For fill vertices, we typically have 2 int16 values (x,y) = 4 bytes per vertex
                    std::size_t effectiveStride = stride;
                    if (effectiveStride == 0) {
                        // Default stride for position data (2 x int16)
                        effectiveStride = 4;
                    }
                    std::size_t vertexSize = effectiveStride;
                    std::size_t totalSize = totalVertices * vertexSize;

                    impl->vertexData.resize(totalSize);
                    impl->vertexStride = vertexSize;
                    impl->vertexSize = totalSize;
                    impl->vertexType = attrType;

                    // Copy vertex data with proper stride
                    const uint8_t* srcBytes = static_cast<const uint8_t*>(rawData);
                    for (std::size_t i = 0; i < totalVertices; ++i) {
                        std::size_t srcOffset = (vertexOffset + i) * effectiveStride + offset;
                        std::size_t dstOffset = i * vertexSize;

                        // Check both source and destination bounds
                        if (srcOffset + vertexSize <= rawSize && dstOffset + vertexSize <= totalSize) {
                            std::memcpy(impl->vertexData.data() + dstOffset,
                                      srcBytes + srcOffset,
                                      vertexSize);
                        }
                    }

            } else if (!attr.getRawData().empty()) {
                // Use the raw data directly
                impl->vertexData = attr.getRawData();
                impl->vertexStride = attr.getStride();
                impl->vertexSize = impl->vertexData.size();
                impl->vertexType = attr.getDataType();

            } else {
            }
        } else {
        }

        impl->needsVertexExtraction = false;

        if (impl->vertexData.empty()) {
        }
    }

    // Upload vertex data to GPU
    if (!impl->vertexData.empty() && impl->vertexData.size() > 0) {

        // Release old buffer if it exists (safely)
        if (impl->vertexBuffer) {
            WGPUBuffer oldBuffer = impl->vertexBuffer;
            impl->vertexBuffer = nullptr;
            wgpuBufferRelease(oldBuffer);
        }

        // Ensure buffer size is at least 4 bytes (minimum for WebGPU)
        std::size_t bufferSize = std::max(impl->vertexData.size(), std::size_t(4));

        // Create vertex buffer
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView vertexLabel = {"Vertex Buffer", strlen("Vertex Buffer")};
        bufferDesc.label = vertexLabel;
        bufferDesc.size = bufferSize;
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 0; // Don't map at creation

        impl->vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

        if (impl->vertexBuffer) {
            // Use queue write instead of mapping
            auto& webgpuContext = static_cast<webgpu::Context&>(context);
            WGPUDevice contextDevice = static_cast<WGPUDevice>(webgpuContext.getBackend().getDevice());
            WGPUQueue queue = wgpuDeviceGetQueue(contextDevice);
            if (queue) {
                // Write the actual data (padded with zeros if needed)
                if (impl->vertexData.size() < bufferSize) {
                    // Need to pad with zeros
                    std::vector<uint8_t> paddedData(bufferSize, 0);
                    std::memcpy(paddedData.data(), impl->vertexData.data(), impl->vertexData.size());
                    wgpuQueueWriteBuffer(queue, impl->vertexBuffer, 0, paddedData.data(), bufferSize);
                } else {
                    wgpuQueueWriteBuffer(queue, impl->vertexBuffer, 0, impl->vertexData.data(), impl->vertexData.size());
                }

                // Debug vertex logging disabled to prevent heap corruption
                // String concatenation in multi-threaded context can cause memory issues
            } else {
            }
        } else {
        }
    } else {
    }

    // Upload index data to GPU
    if (impl->indexVector && impl->indexVector->elements() > 0) {
        std::size_t indexSize = impl->indexVector->bytes();

        // Release old buffer if it exists (safely)
        if (impl->indexBuffer) {
            WGPUBuffer oldBuffer = impl->indexBuffer;
            impl->indexBuffer = nullptr;
            wgpuBufferRelease(oldBuffer);
        }

        // Create index buffer
        const void* indexData = impl->indexVector->data();

        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView indexLabel = {"Index Buffer", strlen("Index Buffer")};
        bufferDesc.label = indexLabel;
        bufferDesc.size = indexSize;
        bufferDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 0; // Don't map at creation

        impl->indexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);

        if (impl->indexBuffer) {
            // Use queue write instead of mapping
            auto& webgpuContext = static_cast<webgpu::Context&>(context);
            WGPUDevice contextDevice = static_cast<WGPUDevice>(webgpuContext.getBackend().getDevice());
            WGPUQueue queue = wgpuDeviceGetQueue(contextDevice);
            if (queue) {
                wgpuQueueWriteBuffer(queue, impl->indexBuffer, 0, indexData, indexSize);
            } else {
            }
        } else {
        }
    } else {
    }

    // Upload textures
    uploadTextures(webgpuUploadPass);


}

void Drawable::uploadTextures(UploadPass&) const noexcept {
    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
        }
    }
}

void Drawable::draw(PaintParameters& parameters) const {

    if (!getEnabled()) {
        return;
    }

    // Get the render pass
    if (!parameters.renderPass) {
        return;
    }

    // Get the actual WebGPU render pass encoder
    auto* webgpuRenderPass = static_cast<webgpu::RenderPass*>(parameters.renderPass.get());
    if (!webgpuRenderPass) {
        return;
    }

    WGPURenderPassEncoder renderPassEncoder = webgpuRenderPass->getEncoder();
    if (!renderPassEncoder) {
        return;
    }

    // Update uniform buffer with current projection matrix
    if (impl->uniformBuffer) {
        auto& context = static_cast<webgpu::Context&>(parameters.context);
        auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
        WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
        WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

        if (device && queue) {
            // Get the appropriate matrix for transformation
            mat4 tileMatrix;

            // For debugging: Try using just the projection matrix to see if vertices appear
            static bool useSimpleProjection = true;

            if (useSimpleProjection) {
                // Create a simple orthographic projection for testing
                // The vertex data seems to use a larger range (0-16384 or more)
                // Let's use a wider range to capture all geometry
                float range = 32768.0f;  // Wider range to capture all vertices

                // Initialize to identity
                for (int i = 0; i < 16; i++) tileMatrix[i] = 0.0f;

                // Column 0: X axis scaling
                tileMatrix[0] = 2.0f / range;  // Scale X

                // Column 1: Y axis scaling (flip for Y-down to Y-up)
                tileMatrix[5] = -2.0f / range; // Scale Y (negative to flip)

                // Column 2: Z axis (no change)
                tileMatrix[10] = 1.0f;

                // Column 3: Translation - center the range
                tileMatrix[12] = -1.0f;  // Translate X: map 0 -> -1
                tileMatrix[13] = 1.0f;   // Translate Y: map 0 -> 1 (after flip)
                tileMatrix[14] = 0.0f;   // No Z translation

                // W component (homogeneous coordinate)
                tileMatrix[15] = 1.0f;

                // Debug: Print what we just set

            } else if (const auto& tileid = getTileID()) {
                // Get the tile-specific transformation matrix
                mat4 tileTransform;
                parameters.state.matrixFor(tileTransform, tileid->toUnwrapped());

                // Apply drawable origin if it exists
                if (const auto& orig = getOrigin(); orig.has_value()) {
                    matrix::translate(tileTransform, tileTransform, orig->x, orig->y, 0);
                }

                // Compute final MVP: Projection * View * Model
                const mat4& projMatrix = parameters.transformParams.projMatrix;
                matrix::multiply(tileMatrix, projMatrix, tileTransform);
            } else {
                // Fallback to projection matrix if no tile
                tileMatrix = parameters.transformParams.projMatrix;
            }

            // Log matrix values for debugging - show full matrix
            static int matrixLogCount = 0;
            if (matrixLogCount++ < 5) {  // Only log first few matrices to avoid spam
                // for (int i = 0; i < 4; i++) {
                //               std::to_string(tileMatrix[i*4+2]) + ", " +
                // }
            }

            // Convert to column-major format for WebGPU (mat4 is row-major in MapLibre)
            // Convert from double (mat4) to float for WebGPU
            // mat4 is std::array<double, 16> but WebGPU needs float[16]
            float matrix[16];
            for (int i = 0; i < 16; i++) {
                matrix[i] = static_cast<float>(tileMatrix[i]);
            }

            // Debug: verify conversion
            static int debugCount = 0;
            if (debugCount++ < 5) {

            }

            // Note: WebGPU uses depth range [0, 1] vs OpenGL's [-1, 1]
            // But MapLibre's matrices might already account for this

            // Update the uniform buffer
            wgpuQueueWriteBuffer(queue, impl->uniformBuffer, 0, matrix, sizeof(matrix));

            // Debug logging removed to prevent heap corruption in multi-threaded environment
        }
    }

    // Get pipeline from shader if we don't have it yet
    if (!impl->pipeline) {
        if (!shader) {
            static int noShaderCount = 0;
            if (noShaderCount++ < 5) {
                mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No shader set for drawable");
            }
            return;
        }

        static int shaderCheckCount = 0;
        if (shaderCheckCount++ < 5) {
            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Attempting to get pipeline from shader");
        }
        // Verify it's a WebGPU shader by checking the type name
        if (shader->typeName() != "WebGPU") {
            static int wrongTypeCount = 0;
            if (wrongTypeCount++ < 5) {
                std::string typeName(shader->typeName());
                mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Shader type is '" + typeName + "', expected 'WebGPU'");
            }
            return;
        }

        // Use dynamic_pointer_cast for safer casting with RTTI
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            // Store the pipeline reference - we don't own it, the shader does
            impl->pipeline = webgpuShader->getPipeline();
            if (!impl->pipeline) {
                // Pipeline not created yet or creation failed
                return;
            }
        } else {
            return;
        }
    }

    // Check if we have a valid pipeline
    if (!impl->pipeline) {
        static int noPipelineCount = 0;
        if (noPipelineCount++ < 5) {
            mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No pipeline available for drawable");
        }
        return;
    }

    // Set the pipeline

    wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipeline);

    // Continue with rest of drawing even without pipeline for debugging

    // Bind vertex buffer
    if (impl->vertexBuffer) {
        // Log first vertex position for debugging (assuming int16x2 format)
        if (impl->vertexData.size() >= 4) {
            // int16_t x = *reinterpret_cast<const int16_t*>(impl->vertexData.data());
            // int16_t y = *reinterpret_cast<const int16_t*>(impl->vertexData.data() + 2);
        }
        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, impl->vertexBuffer, 0, impl->vertexData.size());
    } else {
    }

    // Bind index buffer if available
    if (impl->indexBuffer && impl->indexVector) {
        // For now, we assume 16-bit indices
        WGPUIndexFormat indexFormat = WGPUIndexFormat_Uint16;
        wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, impl->indexBuffer, indexFormat, 0, impl->indexVector->bytes());
    } else {

    }

// Bind uniform buffers and textures via bind group
    if (impl->bindGroup) {
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    } else {
    }

    // Draw
    if (impl->indexBuffer && impl->indexVector) {
        // Draw indexed geometry
        uint32_t indexCount = static_cast<uint32_t>(impl->indexVector->elements());



        wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, 0, 0, 0);
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);


        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
    } else {
        static int noDataLogCount = 0;
        if (noDataLogCount++ < 5) {
        }
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexVector = std::move(indices);
    // Note: DrawSegments are handled differently in WebGPU
    // We'll use the entire index buffer for now
    (void)segments;

    // Don't call buildWebGPUPipeline here - it will be called by the caller if needed
    // This avoids redundant calls and potential recursion issues
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType) {
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    if (count > 0) {
        impl->vertexStride = impl->vertexData.size() / count;

        // Log the first few vertices to understand the coordinate range
        if (impl->vertexData.size() >= 4) {

        }
    }
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void Drawable::setEnableColor(bool value) {
    impl->colorEnabled = value;
}

void Drawable::setColorMode(const gfx::ColorMode& value) {
    impl->colorMode = value;
}

void Drawable::setEnableDepth(bool value) {
    impl->depthEnabled = value;
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    impl->depthMask = value;
}

void Drawable::setDepthModeFor3D(const gfx::DepthMode& value) {
    // Set depth mode for 3D rendering
    impl->depthEnabled = value.func != gfx::DepthFunctionType::Never;
    impl->depthMask = value.mask;
}

void Drawable::setStencilModeFor3D(const gfx::StencilMode& value) {
    // Stencil state will be configured in the pipeline
    // Store the value for pipeline creation
    (void)value;
}

void Drawable::setLineWidth([[maybe_unused]] int32_t value) {
    // WebGPU doesn't support line width directly
    // This might need to be handled in the shader
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& value) {
    impl->cullFaceMode = value;
}

void Drawable::setVertexAttrId(std::size_t id) {
    impl->vertexAttrId = id;

}

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
                                     std::size_t vertexCount,
                                     gfx::DrawMode mode,
                                     gfx::IndexVectorBasePtr indexes,
                                     const SegmentBase* segments,
                                     std::size_t segmentCount) {

    // Store the vertex attributes (base class method)
    gfx::Drawable::setVertexAttributes(std::move(vertices));

    // Store vertex count
    impl->vertexCount = vertexCount;

    // Handle segments - check for nullptr
    std::vector<UniqueDrawSegment> drawSegs;
    if (segments && segmentCount > 0) {
        drawSegs.reserve(segmentCount);
        for (std::size_t i = 0; i < segmentCount; ++i) {
            const auto& seg = segments[i];
            drawSegs.push_back(std::make_unique<DrawSegment>(mode, SegmentBase{
                seg.vertexOffset,
                seg.indexOffset,
                seg.vertexLength,
                seg.indexLength,
                seg.sortKey
            }));
        }
    }

    // Set the index data with segments
    setIndexData(std::move(indexes), std::move(drawSegs));

    // Mark that we need to extract vertex data from attributes during upload
    impl->needsVertexExtraction = true;


    buildWebGPUPipeline();


}

void Drawable::buildWebGPUPipeline() noexcept {

    // Clear the pipeline reference (but don't release it - it's owned by the shader)
    impl->pipeline = nullptr;

    // We need a shader to create the pipeline
    if (!shader) {
        return;
    }

    // Cast to WebGPU shader program
    // The shader is a shared_ptr<gfx::ShaderProgramBase> which actually contains a webgpu::ShaderProgram
    // We use static_pointer_cast because RTTI is disabled
    if (!shader) {
        return;
    }

    // Verify it's a WebGPU shader by checking the type name
    if (shader->typeName() != "WebGPU") {
        return;
    }

    auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
    if (!webgpuShader) {
        return;
    }

    // Use the pre-built pipeline from the shader program
    impl->pipeline = webgpuShader->getPipeline();
    if (impl->pipeline) {

        // Also get the bind group layout for creating bind groups
        WGPUBindGroupLayout bindGroupLayout = webgpuShader->getBindGroupLayout();
        if (bindGroupLayout) {
            // Create bind group for uniforms
            createBindGroup(bindGroupLayout);
        }
    } else {
    }
}

void Drawable::createBindGroup(WGPUBindGroupLayout layout) noexcept {
    if (!layout) {
        return;
    }
    
    // Release old bind group if it exists (safely)
    if (impl->bindGroup) {
        WGPUBindGroup oldGroup = impl->bindGroup;
        impl->bindGroup = nullptr;
        wgpuBindGroupRelease(oldGroup);
    }

    // Store the layout for creating bind group during upload
    // Note: We don't own the layout - it's owned by the shader
    impl->bindGroupLayout = layout;
}



} // namespace webgpu
} // namespace mbgl
