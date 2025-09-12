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

Drawable::Drawable(std::string name)
    : gfx::Drawable(std::move(name)),
      impl(std::make_unique<Impl>()) {
    // Uniform buffers are initialized in Impl constructor
}

Drawable::~Drawable() {
    // Clean up WebGPU resources
    if (impl->vertexBuffer) {
        // wgpuBufferDestroy(impl->vertexBuffer);
    }
    if (impl->indexBuffer) {
        // wgpuBufferDestroy(impl->indexBuffer);
    }
    if (impl->pipeline) {
        // wgpuRenderPipelineRelease(impl->pipeline);
    }
    if (impl->bindGroup) {
        // wgpuBindGroupRelease(impl->bindGroup);
    }
}

void Drawable::upload(gfx::UploadPass& uploadPass) {

    auto& webgpuUploadPass = static_cast<webgpu::UploadPass&>(uploadPass);
    auto& gfxContext = webgpuUploadPass.getContext();
    auto& context = static_cast<webgpu::Context&>(gfxContext);
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());

    if (!device) {
        Log::Warning(Event::General, "No device available in upload pass");
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
            Log::Warning(Event::General, "Failed to create bind group");
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
                    auto type = attr.getSharedType();


                    // Get the raw bytes from the shared data
                    const void* rawData = sharedData->getRawData();
                    std::size_t rawSize = sharedData->getRawSize();

                    // Calculate total size needed
                    std::size_t totalVertices = impl->vertexCount;
                    // For fill vertices, we typically have 2 int16 values (x,y) = 4 bytes per vertex
                    std::size_t vertexSize = stride;
                    if (vertexSize == 0) {
                        // Default stride for position data (2 x int16)
                        vertexSize = 4;
                    }
                    std::size_t totalSize = totalVertices * vertexSize;

                    impl->vertexData.resize(totalSize);
                    impl->vertexStride = vertexSize;
                    impl->vertexSize = totalSize;
                    impl->vertexType = type;

                    // Copy vertex data with proper stride
                    const uint8_t* srcBytes = static_cast<const uint8_t*>(rawData);
                    for (std::size_t i = 0; i < totalVertices; ++i) {
                        std::size_t srcOffset = (vertexOffset + i) * stride + offset;
                        std::size_t dstOffset = i * vertexSize;

                        if (srcOffset + vertexSize <= rawSize) {
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
            Log::Warning(Event::General, "Failed to extract vertex data from attributes");
        }
    }

    // Upload vertex data to GPU
    if (!impl->vertexData.empty() && impl->vertexData.size() > 0) {

        // Release old buffer if it exists
        if (impl->vertexBuffer) {
            wgpuBufferRelease(impl->vertexBuffer);
            impl->vertexBuffer = nullptr;
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

                // Log first few vertices for debugging
                if (impl->vertexData.size() >= 12) {
                    const int16_t* vertices = reinterpret_cast<const int16_t*>(impl->vertexData.data());
                    std::string vertexInfo = "Vertices for " + getName() + ":\n";
                    for (int i = 0; i < std::min(3, static_cast<int>(impl->vertexData.size() / 4)); i++) {
                        float x = static_cast<float>(vertices[i * 2]) / 8192.0f;
                        float y = static_cast<float>(vertices[i * 2 + 1]) / 8192.0f;
                        vertexInfo += "  V" + std::to_string(i) + ": raw=[" +
                                     std::to_string(vertices[i * 2]) + ", " +
                                     std::to_string(vertices[i * 2 + 1]) + "], normalized=[" +
                                     std::to_string(x) + ", " + std::to_string(y) + "]\n";
                    }
                }
            } else {
                Log::Error(Event::General, "Failed to get queue for writing vertex buffer");
            }
        } else {
            Log::Error(Event::General, "Failed to create vertex buffer");
        }
    } else {
    }

    // Upload index data to GPU
    if (impl->indexVector && impl->indexVector->elements() > 0) {
        std::size_t indexSize = impl->indexVector->bytes();

        // Release old buffer if it exists
        if (impl->indexBuffer) {
            wgpuBufferRelease(impl->indexBuffer);
            impl->indexBuffer = nullptr;
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
                Log::Error(Event::General, "Failed to get queue for writing index buffer");
            }
        } else {
            Log::Error(Event::General, "Failed to create index buffer");
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

                          ", [12]=" + std::to_string(tileMatrix[12]) +
                          ", [13]=" + std::to_string(tileMatrix[13]) +
            } else if (const auto& tileID = getTileID()) {
                // Get the tile-specific transformation matrix
                mat4 tileTransform;
                parameters.state.matrixFor(tileTransform, tileID->toUnwrapped());

                // Apply drawable origin if it exists
                if (const auto& origin = getOrigin(); origin.has_value()) {
                    matrix::translate(tileTransform, tileTransform, origin->x, origin->y, 0);
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
                for (int i = 0; i < 4; i++) {

                              std::to_string(tileMatrix[i*4+2]) + ", " +
                              std::to_string(tileMatrix[i*4+3]) + "]");
                }
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

            // Log matrix values for debugging (first drawable only)
            static bool loggedMatrix = false;
            if (!loggedMatrix && getName().find("fill") != std::string::npos) {
                loggedMatrix = true;
                std::string matrixInfo = "Matrix for " + getName() + " (column-major storage):\n";
                const float* matrixData = reinterpret_cast<const float*>(&matrix);
                // Print as rows for readability, but data is stored column-major
                for (int row = 0; row < 4; row++) {
                    matrixInfo += "  Row " + std::to_string(row) + ": [";
                    for (int col = 0; col < 4; col++) {
                        // Column-major: element at row r, column c is at index c*4 + r
                        matrixInfo += std::to_string(matrixData[col * 4 + row]);
                        if (col < 3) matrixInfo += ", ";
                    }
                    matrixInfo += "]\n";
                }

                // Test transform a sample vertex to see where it ends up
                if (impl->vertexData.size() >= 4) {
                    const int16_t* vertices = reinterpret_cast<const int16_t*>(impl->vertexData.data());
                    float x = static_cast<float>(vertices[0]);
                    float y = static_cast<float>(vertices[1]);

                    // Apply matrix transformation (column-major order)
                    // For column-major: column i is at indices [i*4, i*4+1, i*4+2, i*4+3]
                    float tx = matrixData[0] * x + matrixData[4] * y + matrixData[8] * 0 + matrixData[12];
                    float ty = matrixData[1] * x + matrixData[5] * y + matrixData[9] * 0 + matrixData[13];
                    float tz = matrixData[2] * x + matrixData[6] * y + matrixData[10] * 0 + matrixData[14];
                    float tw = matrixData[3] * x + matrixData[7] * y + matrixData[11] * 0 + matrixData[15];


                    // Perspective divide to get NDC
                    if (tw != 0) {
                        tx /= tw;
                        ty /= tw;
                        tz /= tw;
                    }


                }
            }
        }
    }

    // Get pipeline from shader if we don't have it yet
    if (!impl->pipeline && shader) {
        // Verify it's a WebGPU shader by checking the type name
        if (shader->typeName() != "WebGPU") {
            Log::Error(Event::General, "Shader is not a WebGPU shader, type: " + std::string(shader->typeName()));
            return;
        }

        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            impl->pipeline = webgpuShader->getPipeline();
            if (impl->pipeline) {
                uintptr_t addr = reinterpret_cast<uintptr_t>(impl->pipeline);

            } else {
                Log::Warning(Event::General, "Shader's getPipeline() returned null");
            }
        }
    }

    // Set the pipeline
    if (!impl->pipeline) {
        Log::Warning(Event::General, "WebGPU Drawable: No pipeline available, cannot draw");
        return;
    }

    // Additional safety check for pipeline validity
    uintptr_t pipelineAddr = reinterpret_cast<uintptr_t>(impl->pipeline);
    if (pipelineAddr < 0x1000) { // Likely a bad pointer
        Log::Error(Event::General, "WebGPU Drawable: Pipeline pointer is invalid (address: 0x" +
                    std::to_string(pipelineAddr) + ")");
        impl->pipeline = nullptr; // Clear the bad pointer
        return;
    }

    // Set the pipeline

    wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipeline);

    // Continue with rest of drawing even without pipeline for debugging

    // Bind vertex buffer
    if (impl->vertexBuffer) {
        // Log first vertex position for debugging (assuming int16x2 format)
        if (impl->vertexData.size() >= 4) {
            int16_t x = *reinterpret_cast<const int16_t*>(impl->vertexData.data());
            int16_t y = *reinterpret_cast<const int16_t*>(impl->vertexData.data() + 2);
        }
        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, impl->vertexBuffer, 0, impl->vertexData.size());
    } else {
        Log::Warning(Event::General, "WebGPU Drawable: No vertex buffer to bind");
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

        // Log draw call details for first few drawables
        static int drawCallLogCount = 0;
        if (drawCallLogCount++ < 10) {

        }

        wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, 0, 0, 0);
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);

    static int drawCallLogCount = 0;
        if (drawCallLogCount++ < 10) {
        }

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
            const int16_t* vertices = reinterpret_cast<const int16_t*>(impl->vertexData.data());
                      ", y=" + std::to_string(vertices[1]));
            if (impl->vertexData.size() >= 8) {
            }
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

void Drawable::setLineWidth(int32_t value) {
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
        Log::Warning(Event::General, "No shader set for drawable, skipping pipeline creation");
        return;
    }

    // Cast to WebGPU shader program
    // The shader is a shared_ptr<gfx::ShaderProgramBase> which actually contains a webgpu::ShaderProgram
    // We use static_pointer_cast because RTTI is disabled
    if (!shader) {
        Log::Error(Event::General, "Shader is null!");
        return;
    }

    // Verify it's a WebGPU shader by checking the type name
    if (shader->typeName() != "WebGPU") {
        Log::Error(Event::General, "Shader is not a WebGPU shader, type: " + std::string(shader->typeName()));
        return;
    }

    auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
    if (!webgpuShader) {
        Log::Error(Event::General, "Failed to cast shader to WebGPU type");
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
        Log::Warning(Event::General, "Shader has no pipeline");
    }
}

void Drawable::createBindGroup(WGPUBindGroupLayout layout) noexcept {

    // Release old bind group if it exists
    if (impl->bindGroup) {
        wgpuBindGroupRelease(impl->bindGroup);
        impl->bindGroup = nullptr;
    }

    // Store the layout for creating bind group during upload
    impl->bindGroupLayout = layout;

}



} // namespace webgpu
} // namespace mbgl
