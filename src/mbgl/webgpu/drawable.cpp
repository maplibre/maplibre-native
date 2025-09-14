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
#include <mbgl/shaders/segment.hpp>
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
    
    // Uniform buffers are managed by UniformBufferArray
    
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

    // Store the bind group layout if available for later bind group creation
    if (impl->bindGroupLayout && !impl->bindGroup && shader) {
        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (webgpuShader) {
            impl->bindGroupLayout = webgpuShader->getBindGroupLayout();
        }

    }
    // Bind group will be created during draw() using the UniformBufferArray

    // Extract vertex data from attributes if needed
    if (impl->needsVertexExtraction && vertexAttributes) {
        // For WebGPU, we need to interleave all vertex attributes into a single buffer
        // Get the shader to determine the expected vertex layout
        if (!shader) {
            impl->needsVertexExtraction = false;
            return;
        }

        auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
        if (!webgpuShader) {
            impl->needsVertexExtraction = false;
            return;
        }

        // Get the attribute infos from the shader
        const auto& attributeInfos = webgpuShader->getAttributeInfos();
        if (attributeInfos.empty()) {
            impl->needsVertexExtraction = false;
            return;
        }

        // Calculate the total vertex stride and attribute strides
        std::vector<std::size_t> attrStrides;
        std::size_t totalStride = 0;
        for (const auto& attrInfo : attributeInfos) {
            std::size_t stride = 0;
            switch (attrInfo.dataType) {
                case gfx::AttributeDataType::Byte: stride = 1; break;
                case gfx::AttributeDataType::UByte: stride = 1; break;
                case gfx::AttributeDataType::Short: stride = 2; break;
                case gfx::AttributeDataType::UShort: stride = 2; break;
                case gfx::AttributeDataType::Int: stride = 4; break;
                case gfx::AttributeDataType::UInt: stride = 4; break;
                case gfx::AttributeDataType::Float: stride = 4; break;
                case gfx::AttributeDataType::Byte2: stride = 2; break;
                case gfx::AttributeDataType::UByte2: stride = 2; break;
                case gfx::AttributeDataType::Short2: stride = 4; break;
                case gfx::AttributeDataType::UShort2: stride = 4; break;
                case gfx::AttributeDataType::Int2: stride = 8; break;
                case gfx::AttributeDataType::UInt2: stride = 8; break;
                case gfx::AttributeDataType::Float2: stride = 8; break;
                case gfx::AttributeDataType::Byte4: stride = 4; break;
                case gfx::AttributeDataType::UByte4: stride = 4; break;
                case gfx::AttributeDataType::Short4: stride = 8; break;
                case gfx::AttributeDataType::UShort4: stride = 8; break;
                case gfx::AttributeDataType::Int4: stride = 16; break;
                case gfx::AttributeDataType::UInt4: stride = 16; break;
                case gfx::AttributeDataType::Float4: stride = 16; break;
                case gfx::AttributeDataType::Int3: stride = 12; break;
                case gfx::AttributeDataType::UInt3: stride = 12; break;
                case gfx::AttributeDataType::Float3: stride = 12; break;
                default: stride = 8; break;
            }
            attrStrides.push_back(stride);
            totalStride += stride;
        }

        if (totalStride == 0 || impl->vertexCount == 0) {
            impl->needsVertexExtraction = false;
            return;
        }

        // Resize vertex data buffer
        impl->vertexData.resize(impl->vertexCount * totalStride);
        impl->vertexStride = totalStride;

        // Interleave data from all attributes
        for (std::size_t vertexIndex = 0; vertexIndex < impl->vertexCount; ++vertexIndex) {
            std::size_t dstOffset = 0;

            for (std::size_t attrIndex = 0; attrIndex < attributeInfos.size(); ++attrIndex) {
                const auto& attrInfo = attributeInfos[attrIndex];
                std::size_t attrStride = attrStrides[attrIndex];

                // Find the corresponding vertex attribute in the drawable
                const auto& drawableAttrPtr = vertexAttributes->get(attrInfo.index);
                if (!drawableAttrPtr) continue;

                const auto& drawableAttr = *drawableAttrPtr;

                // Get the raw data for this attribute
                const auto& rawData = drawableAttr.getRawData();
                if (rawData.empty()) continue;

                // Calculate offset for this vertex in the raw data
                std::size_t vertexOffset = vertexIndex * drawableAttr.getStride();
                if (vertexOffset + attrStride > rawData.size()) continue;

                // Copy to the interleaved buffer
                std::memcpy(impl->vertexData.data() + (vertexIndex * totalStride) + dstOffset, 
                          rawData.data() + vertexOffset, attrStride);
                dstOffset += attrStride;
            }
        }

        impl->needsVertexExtraction = false;
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
    static int drawCallCount = 0;
    if (drawCallCount++ < 10) {
        mbgl::Log::Info(mbgl::Event::Render, "WebGPU Drawable::draw() called for: " + getName());
    }

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
        static int noEncoderCount = 0;
        if (noEncoderCount++ < 5) {
            mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No render pass encoder available");
        }
        return;
    }

    // Get WebGPU context and device
    auto& context = static_cast<webgpu::Context&>(parameters.context);
    auto& backend = static_cast<webgpu::RendererBackend&>(context.getBackend());
    WGPUDevice device = static_cast<WGPUDevice>(backend.getDevice());
    WGPUQueue queue = static_cast<WGPUQueue>(backend.getQueue());

    if (device && queue) {
        // Call bind on UniformBufferArray to ensure buffers are ready (like Metal does)
        impl->uniformBuffers.bind(*parameters.renderPass);

        // Create bind group from UniformBufferArray if needed
        if (!impl->bindGroup && shader) {
            // Get the shader program
            auto webgpuShader = std::static_pointer_cast<mbgl::webgpu::ShaderProgram>(shader);
            if (webgpuShader) {
                WGPUBindGroupLayout layout = webgpuShader->getBindGroupLayout();
                if (layout) {


                        // Create bind group entries from UniformBufferArray
                        std::vector<WGPUBindGroupEntry> entries;

                        // Check the UniformBufferArray for actual uniform buffers
                        // The layer tweakers populate these with the correct data
                        static int debugUniformCount = 0;
                        bool shouldLog = (debugUniformCount++ < 10);

                        // WebGPU shaders now use the same binding indices as Metal/Vulkan
                        // No remapping needed - just use the buffer index directly as the binding index
                        for (size_t i = 0; i < impl->uniformBuffers.allocatedSize(); ++i) {
                            const auto& uniformBuffer = impl->uniformBuffers.get(i);
                            if (uniformBuffer) {
                                const auto* webgpuUniformBuffer = static_cast<const webgpu::UniformBuffer*>(uniformBuffer.get());
                                if (webgpuUniformBuffer && webgpuUniformBuffer->getBuffer()) {
                                    WGPUBindGroupEntry entry = {};
                                    entry.binding = static_cast<uint32_t>(i);
                                    entry.buffer = webgpuUniformBuffer->getBuffer();
                                    entry.offset = 0;
                                    entry.size = webgpuUniformBuffer->getSize();
                                    entries.push_back(entry);

                                    if (shouldLog) {
                                        mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Added uniform buffer at index/binding " +
                                            std::to_string(i) + " with size " + std::to_string(entry.size));
                                    }
                                }
                            }
                        }

                        // Check if we have the minimum required uniform buffers
                        // Fill shaders require at least binding 0 (drawable UBO)
                        // Some may also require binding 1 (props UBO)
                        if (entries.empty()) {
                            static int noUniformBuffersCount = 0;
                            if (noUniformBuffersCount++ < 5) {
                                mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No uniform buffers available in UniformBufferArray for drawable: " + name);
                            }
                            return;  // Can't render without uniform buffers
                        }

                        // If we only have one buffer and the shader expects two, add a dummy for binding 1
                        // This can happen if the props UBO isn't populated by the layer tweaker
                        if (entries.size() == 1 && shouldLog) {
                            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Only one uniform buffer found, shader may expect two");
                        }

                        // Create bind group descriptor
                        WGPUBindGroupDescriptor bgDesc = {};
                        WGPUStringView bgLabel = {"Drawable Bind Group", strlen("Drawable Bind Group")};
                        bgDesc.label = bgLabel;
                        bgDesc.layout = layout;
                        bgDesc.entryCount = entries.size();
                        bgDesc.entries = entries.data();

                        // Create the bind group
                        impl->bindGroup = wgpuDeviceCreateBindGroup(device, &bgDesc);
                        if (impl->bindGroup) {
                            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Successfully created bind group for drawable");
                        } else {
                            mbgl::Log::Error(mbgl::Event::Render, "WebGPU: Failed to create bind group for drawable");
                        }
                } else {
                    mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No bind group layout available from shader");
                }
            } else {
                mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Failed to cast shader to WebGPU shader");
            }
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
        if (shader->typeName() != "WebGPUShader") {
            static int wrongTypeCount = 0;
            if (wrongTypeCount++ < 5) {
                std::string typeName(shader->typeName());
                mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: Shader type is '" + typeName + "', expected 'WebGPUShader'");
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
    static int pipelineSetCount = 0;
    if (pipelineSetCount++ < 5) {
        mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Setting pipeline on render pass encoder");
    }
    wgpuRenderPassEncoderSetPipeline(renderPassEncoder, impl->pipeline);

    // Continue with rest of drawing even without pipeline for debugging

    // Bind vertex buffer
    if (impl->vertexBuffer) {
        // Log first vertex data for debugging (lines have 8 bytes per vertex)
        if (impl->vertexData.size() >= 8) {
            int16_t pos_normal_x = *reinterpret_cast<const int16_t*>(impl->vertexData.data());
            int16_t pos_normal_y = *reinterpret_cast<const int16_t*>(impl->vertexData.data() + 2);
            uint8_t data0 = impl->vertexData[4];
            uint8_t data1 = impl->vertexData[5];
            uint8_t data2 = impl->vertexData[6];
            uint8_t data3 = impl->vertexData[7];

            static int vertexLogCount = 0;
            if (vertexLogCount++ < 5) {
                mbgl::Log::Info(mbgl::Event::Render, "WebGPU: First vertex - pos_normal: (" +
                    std::to_string(pos_normal_x) + ", " + std::to_string(pos_normal_y) +
                    "), data: (" + std::to_string(data0) + ", " + std::to_string(data1) +
                    ", " + std::to_string(data2) + ", " + std::to_string(data3) + ")");

                mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Vertex buffer size: " +
                    std::to_string(impl->vertexData.size()) + " bytes");
            }
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
        static int bindGroupSetCount = 0;
        if (bindGroupSetCount++ < 10) {
            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Setting bind group on render pass encoder");
        }
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, impl->bindGroup, 0, nullptr);
    } else {
        static int noBindGroupCount = 0;
        if (noBindGroupCount++ < 10) {
            mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No bind group available for drawable");
        }
    }

    // Draw
    if (impl->indexBuffer && impl->indexVector) {
        // Draw indexed geometry - loop through segments like Metal does
        static int totalSegmentLogCount = 0;
        if (totalSegmentLogCount++ < 5) {
            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Drawing " + std::to_string(impl->segments.size()) + " segments");
        }
        for (const auto& seg_ : impl->segments) {
            const auto& segment = static_cast<DrawSegment&>(*seg_);
            const auto& mlSegment = segment.getSegment();
            if (mlSegment.indexLength > 0) {
                uint32_t indexCount = mlSegment.indexLength;
                uint32_t indexOffset = mlSegment.indexOffset;
                uint32_t baseVertex = mlSegment.vertexOffset;

                static int segmentDrawCount = 0;
                if (segmentDrawCount++ < 10) {
                    mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Drawing segment with " + 
                        std::to_string(indexCount) + " indices, offset " + std::to_string(indexOffset) + 
                        ", base vertex " + std::to_string(baseVertex));
                }

                wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, 1, indexOffset, baseVertex, 0);
            }
        }
    } else if (impl->vertexBuffer && impl->vertexCount > 0) {
        // Draw non-indexed
        uint32_t vertexCount = static_cast<uint32_t>(impl->vertexCount);

        static int nonIndexedDrawCount = 0;
        if (nonIndexedDrawCount++ < 5) {
            mbgl::Log::Info(mbgl::Event::Render, "WebGPU: Drawing non-indexed with " + std::to_string(vertexCount) + " vertices");
        }

        wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, 1, 0, 0);
    } else {
        static int noDataLogCount = 0;
        if (noDataLogCount++ < 5) {
            mbgl::Log::Warning(mbgl::Event::Render, "WebGPU: No vertex/index data to draw");
        }
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indices, std::vector<UniqueDrawSegment> segments) {
    impl->indexVector = std::move(indices);
    impl->segments = std::move(segments);
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
