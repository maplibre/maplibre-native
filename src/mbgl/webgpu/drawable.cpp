#include <mbgl/webgpu/drawable.hpp>
#include <mbgl/webgpu/drawable_impl.hpp>
#include <mbgl/webgpu/context.hpp>
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
        return;
    }
    
    // Create bind group if we have a stored layout
    if (impl->bindGroupLayout && !impl->bindGroup) {
        Log::Info(Event::General, "Creating bind group in upload pass");
        
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
            Log::Info(Event::General, "Successfully created bind group");
        } else {
            Log::Warning(Event::General, "Failed to create bind group");
        }
    }
    
    // Extract vertex data from attributes if needed
    if (impl->needsVertexExtraction && vertexAttributes) {
        Log::Info(Event::General, "Extracting vertex data from vertex attributes");
        
        // Look for the position attribute (usually index 0)
        vertexAttributes->visitAttributes([this](const gfx::VertexAttribute& attr) {
            if (attr.getIndex() == 0) { // Position attribute
                // Check if this attribute has shared raw data
                if (attr.getSharedRawData()) {
                    auto sharedData = attr.getSharedRawData();
                    auto offset = attr.getSharedOffset();
                    auto vertexOffset = attr.getSharedVertexOffset();
                    auto stride = attr.getSharedStride();
                    auto type = attr.getSharedType();
                    
                    Log::Info(Event::General, "Found shared vertex data - offset: " + std::to_string(offset) + 
                             ", vertexOffset: " + std::to_string(vertexOffset) + ", stride: " + std::to_string(stride));
                    
                    // Get the raw bytes from the shared data
                    const void* rawData = sharedData->getRawData();
                    std::size_t rawSize = sharedData->getRawSize();
                    
                    // Calculate total size needed
                    std::size_t totalVertices = impl->vertexCount;
                    std::size_t vertexSize = stride ? stride : 4; // Default to 4 bytes (2 x int16)
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
                    
                    Log::Info(Event::General, "Extracted " + std::to_string(totalSize) + " bytes of vertex data for " + std::to_string(totalVertices) + " vertices");
                    return; // Found position, stop visiting
                } else if (!attr.getRawData().empty()) {
                    // Use the raw data directly
                    impl->vertexData = attr.getRawData();
                    impl->vertexStride = attr.getStride();
                    impl->vertexSize = impl->vertexData.size();
                    impl->vertexType = attr.getDataType();
                    
                    Log::Info(Event::General, "Using raw vertex data directly - " + std::to_string(impl->vertexData.size()) + " bytes");
                    return; // Found position, stop visiting
                }
            }
        });
        
        impl->needsVertexExtraction = false;
    }
    
    // Upload vertex data to GPU
    if (!impl->vertexData.empty()) {
        // Release old buffer if it exists
        if (impl->vertexBuffer) {
            wgpuBufferRelease(impl->vertexBuffer);
            impl->vertexBuffer = nullptr;
        }
        
        // Create vertex buffer
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView vertexLabel = {"Vertex Buffer", strlen("Vertex Buffer")};
        bufferDesc.label = vertexLabel;
        bufferDesc.size = impl->vertexData.size();
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 1;
        
        impl->vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (impl->vertexBuffer) {
            // Copy data to buffer
            void* mappedData = wgpuBufferGetMappedRange(impl->vertexBuffer, 0, impl->vertexData.size());
            if (mappedData) {
                std::memcpy(mappedData, impl->vertexData.data(), impl->vertexData.size());
                wgpuBufferUnmap(impl->vertexBuffer);
            }
        }
    }
    
    // Upload index data to GPU
    if (impl->indexVector && impl->indexVector->elements() > 0) {
        // Release old buffer if it exists
        if (impl->indexBuffer) {
            wgpuBufferRelease(impl->indexBuffer);
            impl->indexBuffer = nullptr;
        }
        
        // Create index buffer
        const void* indexData = impl->indexVector->data();
        std::size_t indexSize = impl->indexVector->bytes();
        
        WGPUBufferDescriptor bufferDesc = {};
        WGPUStringView indexLabel = {"Index Buffer", strlen("Index Buffer")};
        bufferDesc.label = indexLabel;
        bufferDesc.size = indexSize;
        bufferDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = 1;
        
        impl->indexBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
        
        if (impl->indexBuffer) {
            // Copy data to buffer
            void* mappedData = wgpuBufferGetMappedRange(impl->indexBuffer, 0, indexSize);
            if (mappedData) {
                std::memcpy(mappedData, indexData, indexSize);
                wgpuBufferUnmap(impl->indexBuffer);
            }
        }
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
    Log::Info(Event::General, "WebGPU Drawable::draw called for " + getName());
    
    if (!getEnabled()) {
        Log::Info(Event::General, "Drawable not enabled, returning");
        return;
    }
    
    // Get the render pass
    if (!parameters.renderPass) {
        Log::Info(Event::General, "No render pass, returning");
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
            // Get the projection matrix from state
            mat4 projMatrix;
            parameters.state.getProjMatrix(projMatrix);
            
            // Convert to column-major format for WebGPU (mat4 is row-major in MapLibre)
            float matrix[16];
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    matrix[j * 4 + i] = projMatrix[i * 4 + j];
                }
            }
            
            // Update the uniform buffer
            wgpuQueueWriteBuffer(queue, impl->uniformBuffer, 0, matrix, sizeof(matrix));
        }
    }
    
    // Set the pipeline
    if (impl->pipeline) {
        Log::Info(Event::General, "WebGPU Drawable: Setting pipeline");
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipeline);
    } else {
        Log::Info(Event::General, "WebGPU Drawable: No pipeline available, cannot draw");
        return;
    }
    
    // Bind vertex buffer
    if (impl->vertexBuffer) {
        wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, impl->vertexBuffer, 0, impl->vertexData.size());
    }
    
    // Bind index buffer if available
    if (impl->indexBuffer && impl->indexVector) {
        // For now, we assume 16-bit indices
        WGPUIndexFormat indexFormat = WGPUIndexFormat_Uint16;
        wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, impl->indexBuffer, indexFormat, 0, impl->indexVector->bytes());
    }
    
    // Bind uniform buffers and textures via bind group
    if (impl->bindGroup) {
        Log::Info(Event::General, "WebGPU Drawable: Setting bind group");
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    } else {
        Log::Info(Event::General, "WebGPU Drawable: No bind group to set");
    }
    
    // Draw
    if (impl->indexBuffer && impl->indexVector) {
        // Draw indexed
        uint32_t indexCount = static_cast<uint32_t>(impl->indexVector->elements());
        Log::Info(Event::General, "WebGPU Drawable: Drawing indexed with " + std::to_string(indexCount) + " indices");
        wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, 0, 0, 0);
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);
        Log::Info(Event::General, "WebGPU Drawable: Drawing non-indexed with " + std::to_string(vertexCount) + " vertices");
        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
    } else {
        Log::Info(Event::General, "WebGPU Drawable: No vertex/index data to draw");
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexVector = std::move(indices);
    // Note: DrawSegments are handled differently in WebGPU
    // We'll use the entire index buffer for now
    (void)segments;
    
    // Mark as dirty to rebuild pipeline if needed
    buildWebGPUPipeline();
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType) {
    Log::Info(Event::General, "Drawable::setVertices called with " + std::to_string(count) + " vertices, data size: " + std::to_string(data.size()));
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    if (count > 0) {
        impl->vertexStride = impl->vertexData.size() / count;
        
        // Log the first few vertices to understand the coordinate range  
        if (impl->vertexData.size() >= 4) {
            const int16_t* vertices = reinterpret_cast<const int16_t*>(impl->vertexData.data());
            Log::Info(Event::General, "First vertex: x=" + std::to_string(vertices[0]) + 
                      ", y=" + std::to_string(vertices[1]));
            if (impl->vertexData.size() >= 8) {
                Log::Info(Event::General, "Second vertex: x=" + std::to_string(vertices[2]) + 
                          ", y=" + std::to_string(vertices[3]));
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

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
                                     std::size_t vertexCount,
                                     gfx::DrawMode mode,
                                     gfx::IndexVectorBasePtr indexes,
                                     const SegmentBase* segments,
                                     std::size_t segmentCount) {
    Log::Info(Event::General, "WebGPU Drawable::updateVertexAttributes - vertexCount: " + std::to_string(vertexCount) + ", segmentCount: " + std::to_string(segmentCount));
    
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
    
    Log::Info(Event::General, "Vertex attributes set, will extract data during upload");
    
    buildWebGPUPipeline();
}

void Drawable::buildWebGPUPipeline() noexcept {
    Log::Info(Event::General, "Building WebGPU pipeline for drawable");
    
    // Release old pipeline if it exists
    if (impl->pipeline) {
        wgpuRenderPipelineRelease(impl->pipeline);
        impl->pipeline = nullptr;
    }
    
    // We need a shader to create the pipeline
    if (!shader) {
        Log::Warning(Event::General, "No shader set for drawable, skipping pipeline creation");
        return;
    }
    
    // Cast to WebGPU shader program
    auto* webgpuShader = static_cast<mbgl::webgpu::ShaderProgram*>(shader.get());
    if (!webgpuShader) {
        Log::Error(Event::General, "Failed to cast shader to WebGPU type");
        return;
    }
    
    // Use the pre-built pipeline from the shader program
    impl->pipeline = webgpuShader->getPipeline();
    if (impl->pipeline) {
        Log::Info(Event::General, "Successfully set pipeline from shader");
        
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
    Log::Info(Event::General, "Storing bind group layout for deferred creation");
    
    // Release old bind group if it exists
    if (impl->bindGroup) {
        wgpuBindGroupRelease(impl->bindGroup);
        impl->bindGroup = nullptr;
    }
    
    // Store the layout for creating bind group during upload
    impl->bindGroupLayout = layout;
    Log::Info(Event::General, "Bind group layout stored, will create bind group in upload()");
}



} // namespace webgpu
} // namespace mbgl