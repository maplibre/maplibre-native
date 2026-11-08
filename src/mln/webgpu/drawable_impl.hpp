#pragma once

#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_impl.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/gfx/uniform.hpp>
#include <mln/gfx/uniform_buffer.hpp>
#include <mln/webgpu/uniform_buffer.hpp>
#include <mln/webgpu/render_pass.hpp>
#include <mln/webgpu/upload_pass.hpp>
#include <mln/webgpu/vertex_attribute.hpp>
#include <mln/webgpu/vertex_buffer_resource.hpp>
#include <webgpu/webgpu.h>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/util/mat4.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace mln {
namespace webgpu {

class Drawable::Impl final {
public:
    Impl() = default;
    ~Impl() = default;

    std::vector<UniqueDrawSegment> segments;

    // Vertex description (similar to Metal's vertexDesc/vertexDescHash)
    std::size_t vertexDescHash = 0;

    // Index data (matching Metal's indexes)
    gfx::IndexVectorBasePtr indexes;
    std::size_t vertexCount = 0;
    gfx::AttributeDataType vertexType = gfx::AttributeDataType::Invalid;

    gfx::AttributeBindingArray attributeBindings;
    gfx::AttributeBindingArray instanceBindings;

    // Store dummy vertex buffers to keep them alive
    std::vector<std::unique_ptr<gfx::VertexBufferResource>> dummyVertexBuffers;

    UniformBufferArray uniformBuffers;
    gfx::UniformBufferPtr uboIndexUniform;

    // Render state (matching Metal)
    gfx::DepthMode depthMode = gfx::DepthMode::disabled();
    gfx::StencilMode stencilMode;
    gfx::CullFaceMode cullFaceMode;
    std::size_t vertexAttrId = 0;
    std::size_t stencilModeHash = 0;
    bool stencilModeHashValid = false;

    // Pipeline state (WebGPU equivalent of Metal's pipelineState)
    WGPURenderPipeline pipelineState = nullptr;

    std::optional<gfx::RenderPassDescriptor> renderPassDescriptor;

    // WebGPU-specific state (similar to Metal's depthStencilState)
    WGPUDepthStencilState depthStencilState = {};
    gfx::StencilMode previousStencilMode;

    // The UBO index that uboIndexUniform holds.
    uint32_t uboIndexValue = 0;

    // A bind group and the layout and entries it was built from. It holds a
    // reference on each entry's resource, so a handle it compares can't be
    // freed and reused for another object.
    struct BindGroupRecord {
        uint32_t slot = 0;
        uint32_t group = 0;
        WGPUBindGroupLayout layout = nullptr;
        WGPUBindGroup handle = nullptr;
        std::vector<WGPUBindGroupEntry> entries;

        BindGroupRecord() = default;
        BindGroupRecord(BindGroupRecord&& other) noexcept
            : slot(other.slot),
              group(other.group),
              layout(other.layout),
              handle(std::exchange(other.handle, nullptr)),
              entries(std::exchange(other.entries, {})) {}
        BindGroupRecord& operator=(BindGroupRecord&&) = delete;
        ~BindGroupRecord() { reset(); }

        bool matches(WGPUBindGroupLayout layout_, const std::vector<WGPUBindGroupEntry>& entries_) const {
            return handle && layout == layout_ &&
                   std::equal(entries.begin(),
                              entries.end(),
                              entries_.begin(),
                              entries_.end(),
                              [](const WGPUBindGroupEntry& a, const WGPUBindGroupEntry& b) {
                                  return a.binding == b.binding && a.buffer == b.buffer && a.offset == b.offset &&
                                         a.size == b.size && a.sampler == b.sampler && a.textureView == b.textureView;
                              });
        }

        void assign(uint32_t slot_,
                    uint32_t group_,
                    WGPUBindGroupLayout layout_,
                    WGPUBindGroup handle_,
                    std::vector<WGPUBindGroupEntry> entries_) {
            reset();
            slot = slot_;
            group = group_;
            layout = layout_;
            handle = handle_;
            entries = std::move(entries_);
            for (const auto& entry : entries) {
                if (entry.buffer) {
                    wgpuBufferAddRef(entry.buffer);
                }
                if (entry.sampler) {
                    wgpuSamplerAddRef(entry.sampler);
                }
                if (entry.textureView) {
                    wgpuTextureViewAddRef(entry.textureView);
                }
            }
        }

        void reset() {
            for (const auto& entry : entries) {
                if (entry.buffer) {
                    wgpuBufferRelease(entry.buffer);
                }
                if (entry.sampler) {
                    wgpuSamplerRelease(entry.sampler);
                }
                if (entry.textureView) {
                    wgpuTextureViewRelease(entry.textureView);
                }
            }
            entries.clear();
            if (handle) {
                wgpuBindGroupRelease(handle);
                handle = nullptr;
            }
            layout = nullptr;
        }
    };
    std::vector<BindGroupRecord> bindGroups;
};

// WebGPU-specific DrawSegment inheriting from the base gfx::Drawable::DrawSegment
using DrawSegment = gfx::Drawable::DrawSegment;

} // namespace webgpu
} // namespace mln
