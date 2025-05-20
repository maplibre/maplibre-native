#include <mbgl/gl/upload_pass.hpp>

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/command_encoder.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/gl/texture2d.hpp>

#include <algorithm>

namespace mbgl {
namespace gl {

using namespace platform;

UploadPass::UploadPass(gl::CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_),
      debugGroup(commandEncoder.createDebugGroup(name)) {}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(const void* data,
                                                                                  const std::size_t size,
                                                                                  const gfx::BufferUsageType usage,
                                                                                  bool /*persistent*/) {
    BufferID id = 0;
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    commandEncoder.context.renderingStats().numBuffers++;
    commandEncoder.context.renderingStats().memVertexBuffers += static_cast<int>(size);
    // NOLINTNEXTLINE(performance-move-const-arg)
    UniqueBuffer result{std::move(id), {commandEncoder.context}};
    commandEncoder.context.vertexBuffer = result;
    MBGL_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, size, data, Enum<gfx::BufferUsageType>::to(usage)));
    return std::make_unique<gl::VertexBufferResource>(std::move(result), static_cast<int>(size));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, const void* data, std::size_t size) {
    commandEncoder.context.vertexBuffer = static_cast<gl::VertexBufferResource&>(resource).getBuffer();
    MBGL_CHECK_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(const void* data,
                                                                                std::size_t size,
                                                                                const gfx::BufferUsageType usage,
                                                                                bool /*persistent*/) {
    BufferID id = 0;
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    commandEncoder.context.renderingStats().numBuffers++;
    commandEncoder.context.renderingStats().memIndexBuffers += static_cast<int>(size);
    // NOLINTNEXTLINE(performance-move-const-arg)
    UniqueBuffer result{std::move(id), {commandEncoder.context}};
    commandEncoder.context.bindVertexArray = 0;
    commandEncoder.context.globalVertexArrayState.indexBuffer = result;
    MBGL_CHECK_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, Enum<gfx::BufferUsageType>::to(usage)));
    return std::make_unique<gl::IndexBufferResource>(std::move(result), static_cast<int>(size));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, const void* data, std::size_t size) {
    // Be sure to unbind any existing vertex array object before binding the
    // index buffer so that we don't mess up another VAO
    commandEncoder.context.bindVertexArray = 0;
    commandEncoder.context.globalVertexArrayState.indexBuffer = static_cast<gl::IndexBufferResource&>(resource).buffer;
    MBGL_CHECK_ERROR(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data));
}

struct VertexBufferGL : public gfx::VertexBufferBase {
    ~VertexBufferGL() override = default;

    std::unique_ptr<gfx::VertexBufferResource> resource;
};

namespace {
const std::unique_ptr<gfx::VertexBufferResource> noBuffer;
}
const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vec,
                                                             const gfx::BufferUsageType usage) {
    if (vec) {
        const auto* rawBufPtr = vec->getRawData();
        const auto rawBufSize = static_cast<int>(vec->getRawCount() * vec->getRawSize());

        // If we already have a buffer...
        if (auto* rawData = static_cast<VertexBufferGL*>(vec->getBuffer()); rawData && rawData->resource) {
            auto& resource = static_cast<gl::VertexBufferResource&>(*rawData->resource);

            // If the already-allocated buffer is large enough, we can re-use it
            if (rawBufSize <= resource.getByteSize()) {
                // If the source changed, update the buffer contents
                if (vec->isModifiedAfter(resource.getLastUpdated())) {
                    updateVertexBufferResource(resource, rawBufPtr, rawBufSize);
                    resource.setLastUpdated(vec->getLastModified());
                }
                return rawData->resource;
            }
        }
        // Otherwise, create a new one
        if (rawBufSize > 0) {
            auto buffer = std::make_unique<VertexBufferGL>();
            buffer->resource = createVertexBufferResource(rawBufPtr, rawBufSize, usage, /*persistent=*/false);
            vec->setBuffer(std::move(buffer));
            return static_cast<VertexBufferGL*>(vec->getBuffer())->resource;
        }
    }
    return noBuffer;
}

static std::size_t padSize(std::size_t size, std::size_t padding) {
    return (padding - (size % padding)) % padding;
}
template <typename T>
static std::size_t pad(std::vector<T>& vector, std::size_t size, T value) {
    const auto count = padSize(vector.size(), size);
    vector.insert(vector.end(), count, value);
    return count;
}

gfx::AttributeBindingArray UploadPass::buildAttributeBindings(
    const std::size_t vertexCount,
    const gfx::AttributeDataType vertexType,
    const std::size_t vertexAttributeIndex,
    const std::vector<std::uint8_t>& vertexData,
    const gfx::VertexAttributeArray& defaults,
    const gfx::VertexAttributeArray& overrides,
    const gfx::BufferUsageType usage,
    const std::optional<std::chrono::duration<double>> lastUpdate,
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) {
    MLN_TRACE_FUNC();
    AttributeBindingArray bindings;
    bindings.resize(defaults.allocatedSize());

    constexpr std::size_t align = 16;
    constexpr std::uint8_t padding = 0;

    std::vector<std::uint8_t> allData;

    uint32_t vertexStride = 0;
    if (vertexAttributeIndex != static_cast<std::size_t>(-1) && !vertexData.empty()) {
        allData.reserve(vertexData.size() + (defaults.getTotalSize() + align) * vertexCount);

        // Fill in vertices
        allData.insert(allData.end(), vertexData.begin(), vertexData.end());
        bindings.resize(vertexAttributeIndex + 1);
        vertexStride = static_cast<uint32_t>(vertexData.size() / vertexCount);
        bindings[vertexAttributeIndex] = {
            /*.attribute = */ {vertexType, 0},
            /* vertexStride = */ vertexStride,
            /* vertexBufferResource = */ nullptr, // buffer details established later
            /* vertexOffset = */ 0,
        };
    }

    // For each attribute in the program, with the corresponding default and optional override...
    const auto resolveAttr = [&](const size_t id, auto& defaultAttr, auto& overrideAttr) -> void {
        MLN_TRACE_ZONE(binding);
        auto& effectiveAttr = overrideAttr ? *overrideAttr : defaultAttr;
        const auto& defaultGL = static_cast<const VertexAttributeGL&>(defaultAttr);
        const auto stride = defaultAttr.getStride();
        const auto offset = static_cast<uint32_t>(allData.size());
        const auto index = static_cast<std::size_t>(defaultGL.getIndex());

        bindings.resize(std::max(bindings.size(), index + 1));

        if (const auto& buffer = getBuffer(effectiveAttr.getSharedRawData(), usage)) {
            bindings[index] = {
                /*.attribute = */ {effectiveAttr.getSharedType(), effectiveAttr.getSharedOffset()},
                /*.vertexStride = */ effectiveAttr.getSharedStride(),
                /*.vertexBufferResource = */ buffer.get(),
                /*.vertexOffset = */ effectiveAttr.getSharedVertexOffset(),
            };
            return;
        }

        if (index == vertexAttributeIndex) {
            // already handled
            return;
        }

        if (allData.empty()) {
            allData.reserve(vertexData.size() + (defaults.getTotalSize() + align) * vertexCount);
        }

        // Get the raw data for the values in the desired format
        const auto& rawData = VertexAttributeGL::getRaw(effectiveAttr, defaultGL.getGLType(), lastUpdate);

        if (rawData.size() == stride * vertexCount) {
            // The override provided a value for each vertex, append it as-is
            allData.insert(allData.end(), rawData.begin(), rawData.end());
        } else if (rawData.size() == stride) {
            // We only have one value, append a copy for each vertex
            for (std::size_t i = 0; i < vertexCount; ++i) {
                allData.insert(allData.end(), rawData.begin(), rawData.end());
            }
        } else {
            // something else, the binding is invalid
            // TODO: throw?
            Log::Warning(Event::General,
                         "Got " + util::toString(rawData.size()) + " bytes for attribute '" + util::toString(id) +
                             "' (" + util::toString(defaultGL.getIndex()) + "), expected " + util::toString(stride) +
                             " or " + util::toString(stride * vertexCount));
            return;
        }

        if (overrideAttr) {
            overrideAttr->setDirty(false);
        }

        bindings[index] = {
            /*.attribute = */ {defaultAttr.getDataType(), offset},
            /* vertexStride = */ static_cast<uint32_t>(stride),
            /* vertexBufferResource = */ nullptr, // buffer details established later
            /* vertexOffset = */ 0,
        };

        pad(allData, align, padding);

        // The vertex stride is the sum of the attribute strides
        vertexStride += static_cast<uint32_t>(stride);
    };
    // This version is called when the attribute is available, but isn't being used by the shader
    const auto onMissingAttr = [&](const size_t, auto& missingAttr) -> void {
        missingAttr->setDirty(false);
    };
    defaults.resolve(overrides, resolveAttr, onMissingAttr);

    assert(vertexStride * vertexCount <= allData.size());

    if (!allData.empty()) {
        if (auto vertBuf = createVertexBufferResource(allData.data(), allData.size(), usage, /*persistent=*/false)) {
            // Fill in the buffer in each binding that was generated without its own buffer
            std::for_each(bindings.begin(), bindings.end(), [&](auto& b) {
                if (b && !b->vertexBufferResource) {
                    b->vertexBufferResource = vertBuf.get();
                }
            });

            outBuffers.emplace_back(std::move(vertBuf));
        } else {
            assert(false);
            return {};
        }
    }

    assert(std::all_of(bindings.begin(), bindings.end(), [](const auto& b) { return !b || b->vertexBufferResource; }));

    return bindings;
}

void UploadPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void UploadPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

gfx::Context& UploadPass::getContext() {
    return commandEncoder.context;
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.context;
}

} // namespace gl
} // namespace mbgl
