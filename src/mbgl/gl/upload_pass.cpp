#include <mbgl/gl/upload_pass.hpp>

#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/command_encoder.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/gl/texture_resource.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/gl/texture2d.hpp>
#endif

#include <algorithm>

namespace mbgl {
namespace gl {

using namespace platform;

namespace {

UniqueBuffer createUniqueBuffer(
    gl::Context& mainContext, const void* data, std::size_t size, gfx::BufferUsageType usage, GLenum bufferGlTarget) {
    MLN_TRACE_FUNC();

    // mainContext is the main render thread context and is passed to UniqueBuffer deleter.
    // only the main context deletes resources. Shared contexts only upload resources.

    // Note that we call glBindBuffer instead of setting commandEncoder.context.vertexBuffer
    // This is because this function can be used in shared contexts, e.g. shared EGL contexts.
    // States aren't tracked in shared contexts. When this function is called in the main
    // render thread context then the caller must set commandEncoder.context.vertexBuffer

    BufferID id = 0;
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    MBGL_CHECK_ERROR(glBindBuffer(bufferGlTarget, id));
    MBGL_CHECK_ERROR(glBufferData(bufferGlTarget, size, data, Enum<gfx::BufferUsageType>::to(usage)));

    // NOLINTNEXTLINE(performance-move-const-arg)
    return UniqueBuffer{std::move(id), {mainContext}};
}

void updateUniqueBuffer(const UniqueBuffer& buffer, const void* data, std::size_t size, GLenum bufferGlTarget) {
    MLN_TRACE_FUNC();

    // Similar to createUniqueBuffer the caller must set commandEncoder.context.vertexBuffer when
    // it is run in the main render thread context

    MBGL_CHECK_ERROR(glBindBuffer(bufferGlTarget, buffer.get()));
    MBGL_CHECK_ERROR(glBufferSubData(bufferGlTarget, 0, size, data));
}

} // namespace

UploadPass::UploadPass(gl::CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_),
      debugGroup(commandEncoder.createDebugGroup(name)) {}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(const void* data,
                                                                                  const std::size_t size,
                                                                                  const gfx::BufferUsageType usage,
                                                                                  bool /*persistent*/) {
    MLN_TRACE_FUNC();

    constexpr GLenum bufferGlTarget = GL_ARRAY_BUFFER;
    auto& ctx = commandEncoder.context;
    auto& backend = ctx.getBackend();

    ctx.renderingStats().numBuffers++;
    ctx.renderingStats().memVertexBuffers += static_cast<int>(size);

    if (backend.supportFreeThreadedUpload()) {
        auto result = std::make_unique<gl::VertexBufferResource>(
            [&](int size_, gfx::BufferUsageType usage_, const void* data_) {
                return createUniqueBuffer(ctx, data_, size_, usage_, bufferGlTarget);
            },
            [&](const UniqueBuffer& buffer, int size_, const void* data_) {
                updateUniqueBuffer(buffer, data_, size_, bufferGlTarget);
            },
            static_cast<int>(size));
        result->asyncAlloc(backend.getResourceUploadThreadPool(), static_cast<int>(size), usage, data);
        return result;

    } else {
        UniqueBuffer result = createUniqueBuffer(ctx, data, size, usage, bufferGlTarget);
        ctx.vertexBuffer = result;
        return std::make_unique<gl::VertexBufferResource>(std::move(result), static_cast<int>(size));
    }
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, const void* data, std::size_t size) {
    MLN_TRACE_FUNC();

    constexpr GLenum bufferGlTarget = GL_ARRAY_BUFFER;
    auto& ctx = commandEncoder.context;
    auto& backend = ctx.getBackend();
    auto& glResource = static_cast<gl::VertexBufferResource&>(resource);
    assert(static_cast<int>(size) <= glResource.getByteSize());

    if (backend.supportFreeThreadedUpload()) {
        if (glResource.isAsyncPending()) {
            // This happens if an allocation is allocated and then followed with an update
            // This also happens is an uploaded resource in a previous frame has not been used
            glResource.wait();
        }
        glResource.asyncUpdate(backend.getResourceUploadThreadPool(), static_cast<int>(size), data);
    } else {
        const UniqueBuffer& buffer = glResource.pickBuffer();
        ctx.vertexBuffer = buffer;
        updateUniqueBuffer(buffer, data, size, bufferGlTarget);
    }
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(const void* data,
                                                                                std::size_t size,
                                                                                const gfx::BufferUsageType usage,
                                                                                bool /*persistent*/) {
    MLN_TRACE_FUNC();

    constexpr GLenum bufferGlTarget = GL_ELEMENT_ARRAY_BUFFER;
    auto& ctx = commandEncoder.context;
    auto& backend = ctx.getBackend();

    ctx.renderingStats().numBuffers++;
    ctx.renderingStats().memIndexBuffers += static_cast<int>(size);

    if (backend.supportFreeThreadedUpload()) {
        auto result = std::make_unique<gl::IndexBufferResource>(
            [&](int size_, gfx::BufferUsageType usage_, const void* data_) {
                return createUniqueBuffer(ctx, data_, size_, usage_, bufferGlTarget);
            },
            [&](const UniqueBuffer& buffer, int size_, const void* data_) {
                updateUniqueBuffer(buffer, data_, size_, bufferGlTarget);
            },
            static_cast<int>(size));
        result->asyncAlloc(backend.getResourceUploadThreadPool(), static_cast<int>(size), usage, data);
        return result;

    } else {
        ctx.bindVertexArray = 0;
        ctx.globalVertexArrayState.indexBuffer = 0;
        UniqueBuffer result = createUniqueBuffer(ctx, data, size, usage, bufferGlTarget);
        ctx.globalVertexArrayState.indexBuffer = result;
        return std::make_unique<gl::IndexBufferResource>(std::move(result), static_cast<int>(size));
    }
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, const void* data, std::size_t size) {
    MLN_TRACE_FUNC();

    constexpr GLenum bufferGlTarget = GL_ELEMENT_ARRAY_BUFFER;
    auto& ctx = commandEncoder.context;
    auto& backend = ctx.getBackend();
    auto& glResource = static_cast<gl::IndexBufferResource&>(resource);
    assert(static_cast<int>(size) <= glResource.getByteSize());

    if (backend.supportFreeThreadedUpload()) {
        if (glResource.isAsyncPending()) {
            // This happens if an allocation is allocated and then followed with an update
            // This also happens is an uploaded resource in a previous frame has not been used
            glResource.wait();
        }
        glResource.asyncUpdate(backend.getResourceUploadThreadPool(), static_cast<int>(size), data);
    } else {
        const UniqueBuffer& buffer = glResource.pickBuffer();
        // Be sure to unbind any existing vertex array object before binding the
        // index buffer so that we don't mess up another VAO
        ctx.bindVertexArray = 0;
        ctx.globalVertexArrayState.indexBuffer = buffer;
        updateUniqueBuffer(buffer, data, size, bufferGlTarget);
    }
}

std::unique_ptr<gfx::TextureResource> UploadPass::createTextureResource(const Size size,
                                                                        const void* data,
                                                                        gfx::TexturePixelType format,
                                                                        gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    auto obj = commandEncoder.context.createUniqueTexture(size, format, type);
    auto resource = std::make_unique<gl::TextureResource>(std::move(obj));
    commandEncoder.context.pixelStoreUnpack = {1};
    updateTextureResourceSub(*resource, 0, 0, size, data, format, type);
    // We are using clamp to edge here since OpenGL ES doesn't allow GL_REPEAT
    // on NPOT textures. We use those when the pixelRatio isn't a power of two,
    // e.g. on iPhone 6 Plus.
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    return resource;
}

void UploadPass::updateTextureResource(gfx::TextureResource& resource,
                                       const Size size,
                                       const void* data,
                                       gfx::TexturePixelType format,
                                       gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    updateTextureResourceSub(resource, 0, 0, size, data, format, type);
}

void UploadPass::updateTextureResourceSub(gfx::TextureResource& resource,
                                          const uint16_t xOffset,
                                          const uint16_t yOffset,
                                          const Size size,
                                          const void* data,
                                          gfx::TexturePixelType format,
                                          gfx::TextureChannelDataType type) {
    MLN_TRACE_FUNC();

    auto& ctx = commandEncoder.context;
    assert(ctx.getTexturePool().isUsed(static_cast<gl::TextureResource&>(resource).texture));
    assert(ctx.getTexturePool().desc(static_cast<gl::TextureResource&>(resource).texture).channelType == type);
    assert(ctx.getTexturePool().desc(static_cast<gl::TextureResource&>(resource).texture).pixelFormat == format);
    assert(ctx.getTexturePool().desc(static_cast<gl::TextureResource&>(resource).texture).size.width >=
           xOffset + size.width);
    assert(ctx.getTexturePool().desc(static_cast<gl::TextureResource&>(resource).texture).size.height >=
           yOffset + size.height);

    // Always use texture unit 0 for manipulating it.
    ctx.activeTextureUnit = 0;
    ctx.texture[0] = static_cast<const gl::TextureResource&>(resource).texture;
    MBGL_CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D,
                                     0,
                                     xOffset,
                                     yOffset,
                                     size.width,
                                     size.height,
                                     Enum<gfx::TexturePixelType>::to(format),
                                     Enum<gfx::TextureChannelDataType>::to(type),
                                     data));
}

struct VertexBufferGL : public gfx::VertexBufferBase {
    ~VertexBufferGL() override = default;

    std::unique_ptr<gfx::VertexBufferResource> resource;
};

#if MLN_DRAWABLE_RENDERER
namespace {
const std::unique_ptr<gfx::VertexBufferResource> noBuffer;
}
const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vec,
                                                             const gfx::BufferUsageType usage) {
    MLN_TRACE_FUNC();

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

        overrideAttr->setDirty(false);

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
#endif

void UploadPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void UploadPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

#if MLN_DRAWABLE_RENDERER
gfx::Context& UploadPass::getContext() {
    return commandEncoder.context;
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.context;
}
#endif

} // namespace gl
} // namespace mbgl
