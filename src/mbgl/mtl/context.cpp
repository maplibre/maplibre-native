#include <mbgl/mtl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/drawable_builder.hpp>
#include <mbgl/mtl/layer_group.hpp>
#include <mbgl/mtl/offscreen_texture.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/tile_layer_group.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/mtl/vertex_buffer_resource.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/program_parameters.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/thread_pool.hpp>
#include <mbgl/util/hash.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include <algorithm>
#include <cstring>

namespace mbgl {
namespace mtl {

// Maximum number of vertex attributes, per vertex descriptor
// 31 for Apple2-8, Mac2, per https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
constexpr uint32_t maximumVertexBindingCount = 31;

Context::Context(RendererBackend& backend_)
    : gfx::Context(mtl::maximumVertexBindingCount),
      backend(backend_) {}

Context::~Context() noexcept {
    if (cleanupOnDestruction) {
        backend.getThreadPool().runRenderJobs(true /* closeQueue */);
        performCleanup();

        emptyVertexBuffer.reset();
        tileVertexBuffer.reset();
        tileIndexBuffer.reset();
        clipMaskShader.reset();
        clipMaskDepthStencilState.reset();
        clipMaskPipelineState.reset();
        clipMaskUniformsBuffer.reset();
        stencilStateRenderable = nullptr;

        for (size_t i = 0; i < globalUniformBuffers.allocatedSize(); i++) {
            globalUniformBuffers.set(i, nullptr);
        }

#if !defined(NDEBUG)
        Log::Debug(Event::General, "Rendering Stats:\n" + stats.toString("\n"));
#endif
        assert(stats.isZero());
    }
}

void Context::beginFrame() {
    backend.getThreadPool().runRenderJobs();
}

void Context::endFrame() {}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<CommandEncoder>(*this);
}

BufferResource Context::createBuffer(
    const void* data, std::size_t size, gfx::BufferUsageType, bool isIndexBuffer, bool persistent) const {
    return {const_cast<Context&>(*this), data, size, MTL::ResourceStorageModeShared, isIndexBuffer, persistent};
}

// If enabled, ubsan flags the implicit cast from assigning `NS::String*` to `NS::Object*` here
// as an alignment error.  The ObjC NSString allocations are cast to a C++ object pointer by
// `NS::Object::sendMessage`, and the alignment rules are different.  We assume that, because
// `NS::Object` does not use virtual methods and only serves as a way for C++ code to store
// and pass ObjC `id`s to `objc_msgSend`, this won't cause any real problems.
UniqueShaderProgram Context::createProgram(shaders::BuiltIn shaderID,
                                           std::string name,
                                           const std::string_view source,
                                           const std::string_view vertexName,
                                           const std::string_view fragmentName,
                                           const ProgramParameters& programParameters,
                                           const mbgl::unordered_map<std::string, std::string>& additionalDefines) {
    const auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    // No NSMutableDictionary?
    const auto& programDefines = programParameters.getDefines();
    const auto numDefines = programDefines.size() + additionalDefines.size();

    std::string defineStr;
    std::vector<const NS::Object*> rawDefines;
    rawDefines.reserve(2 * numDefines);
    const auto addDefine = [&rawDefines, &defineStr](const auto& pair) {
        const auto* nsKey = NS::String::string(pair.first.data(), NS::UTF8StringEncoding);
        const auto* nsVal = NS::String::string(pair.second.data(), NS::UTF8StringEncoding);
        rawDefines.insert(std::next(rawDefines.begin(), rawDefines.size() / 2), nsKey);
        rawDefines.insert(rawDefines.end(), nsVal);
        defineStr += "#define " + pair.first + " " + pair.second + "\n";
    };
    std::for_each(programDefines.begin(), programDefines.end(), addDefine);
    std::for_each(additionalDefines.begin(), additionalDefines.end(), addDefine);

    observer->onPreCompileShader(shaderID, gfx::Backend::Type::Metal, defineStr);

    const auto nsDefines = NS::Dictionary::dictionary(
        &rawDefines[numDefines], rawDefines.data(), static_cast<NS::UInteger>(numDefines));
    rawDefines.clear();

    auto options = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    options->setPreprocessorMacros(nsDefines);
    options->setFastMathEnabled(true);
    options->setLanguageVersion(MTL::LanguageVersion2_1);

    // TODO: Compile common code into a `LibraryTypeDynamic` to be used by other shaders
    // instead of duplicating that code in each and every shader compilation.
    // requires a check for iOS 14+
    // options->setLibraryType(MTL::LibraryTypeExecutable);

    // Allows use of the [[invariant]] attribute on position outputs to
    // guarantee that the GPU performs the calculations the same way.
    // requires a check for iOS 14+
    // options->setPreserveInvariance(true);

    // TODO: Allow use of `LibraryOptimizationLevelSize` which "may also reduce compile time"
    // requires a check for iOS 16+
    // options->setOptimizationLevel(MTL::LibraryOptimizationLevelDefault);

    NS::Error* error = nullptr;
    NS::String* nsSource = NS::String::string(source.data(), NS::UTF8StringEncoding);

    const auto& device = backend.getDevice();
    auto library = NS::TransferPtr(device->newLibrary(nsSource, options.get(), &error));
    if (!library || error) {
        const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
        const auto errStr = (errPtr && errPtr[0]) ? ": " + std::string(errPtr) : std::string();
        Log::Error(Event::Shader, name + " compile failed" + errStr);
        observer->onShaderCompileFailed(shaderID, gfx::Backend::Type::Metal, defineStr);
        assert(false);
        return nullptr;
    }

    const auto nsVertName = NS::String::string(vertexName.data(), NS::UTF8StringEncoding);
    MTLFunctionPtr vertexFunction = NS::TransferPtr(library->newFunction(nsVertName));
    if (!vertexFunction) {
        Log::Error(Event::Shader, name + " missing vertex function " + vertexName.data());
        observer->onShaderCompileFailed(shaderID, gfx::Backend::Type::Metal, defineStr);
        assert(false);
        return nullptr;
    }

    // fragment function is optional
    MTLFunctionPtr fragmentFunction;
    if (!fragmentName.empty()) {
        const auto nsFragName = NS::String::string(fragmentName.data(), NS::UTF8StringEncoding);
        fragmentFunction = NS::TransferPtr(library->newFunction(nsFragName));
        if (!fragmentFunction) {
            Log::Error(Event::Shader, name + " missing fragment function " + fragmentName.data());
            observer->onShaderCompileFailed(shaderID, gfx::Backend::Type::Metal, defineStr);
            assert(false);
            return nullptr;
        }
    }

    auto shader = std::make_unique<ShaderProgram>(
        std::move(name), backend, std::move(vertexFunction), std::move(fragmentFunction));
    observer->onPostCompileShader(shaderID, gfx::Backend::Type::Metal, defineStr);

    return shader;
}

MTLTexturePtr Context::createMetalTexture(MTLTextureDescriptorPtr textureDescriptor) const {
    return NS::TransferPtr(backend.getDevice()->newTexture(textureDescriptor.get()));
}

MTLSamplerStatePtr Context::createMetalSamplerState(MTLSamplerDescriptorPtr samplerDescriptor) const {
    return NS::TransferPtr(backend.getDevice()->newSamplerState(samplerDescriptor.get()));
}

void Context::performCleanup() {
    stats.numDrawCalls = 0;
    stats.numFrames++;
    clipMaskUniformsBufferUsed = false;
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent, bool /*ssbo*/) {
    return std::make_shared<UniformBuffer>(
        createBuffer(data, size, gfx::BufferUsageType::StaticDraw, /*isIndexBuffer=*/false, persistent));
}

UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArray>();
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    const auto shaderGroup = shaders.getShaderGroup(name);
    auto shader = shaderGroup ? shaderGroup->getOrCreateShader(*this, {}) : gfx::ShaderProgramBasePtr{};
    return std::static_pointer_cast<gfx::ShaderProgramBase>(std::move(shader));
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    return std::make_shared<LayerGroup>(layerIndex, initialCapacity, name);
}

gfx::Texture2DPtr Context::createTexture2D() {
    return std::make_shared<Texture2D>(*this);
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    return std::make_shared<RenderTarget>(*this, size, type);
}

void Context::resetState(gfx::DepthMode, gfx::ColorMode) {}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& buffer,
                                           const void* data,
                                           std::size_t size,
                                           bool persistent) {
    if (buffer) {
        buffer->update(data, size);
        return false;
    } else {
        buffer = createUniformBuffer(data, size, persistent);
        return true;
    }
}

const BufferResource& Context::getTileVertexBuffer() {
    if (!tileVertexBuffer) {
        const auto vertices = RenderStaticData::tileVertices();
        tileVertexBuffer.emplace(createBuffer(vertices.data(),
                                              vertices.bytes(),
                                              gfx::BufferUsageType::StaticDraw,
                                              /*isIndexBuffer=*/true,
                                              /*persistent=*/true));
    }
    return *tileVertexBuffer;
}

const BufferResource& Context::getTileIndexBuffer() {
    if (!tileIndexBuffer) {
        const auto indexes = RenderStaticData::quadTriangleIndices();
        tileIndexBuffer.emplace(createBuffer(indexes.data(),
                                             indexes.bytes(),
                                             gfx::BufferUsageType::StaticDraw,
                                             /*isIndexBuffer=*/true,
                                             /*persistent=*/true));
    }
    return *tileIndexBuffer;
}

const UniqueVertexBufferResource& Context::getEmptyVertexBuffer() {
    if (!emptyVertexBuffer) {
        // This buffer is bound to vertex attribtue indexes when the uniforms are used instead and
        // shaders are expected not to access the attribute values, but Metal requires a binding.
        // This buffer could also hold a single default value applied for all vertices, in which case
        // it could not be shared (See `MTL::VertexStepFunctionConstant` in the vertex attribute
        // descriptors, but that isn't currently being used.
        constexpr std::size_t size = 32;
        constexpr auto usage = gfx::BufferUsageType::StaticDraw;
        emptyVertexBuffer = std::make_unique<VertexBufferResource>(
            createBuffer(nullptr, size, usage, /*isIndexBuffer=*/false, /*persistent=*/false));
    }
    return emptyVertexBuffer;
}

namespace {
const auto clipMaskStencilMode = gfx::StencilMode{
    /*.test=*/gfx::StencilMode::Always(),
    /*.ref=*/0,
    /*.mask=*/0b11111111,
    /*.fail=*/gfx::StencilOpType::Keep,
    /*.depthFail=*/gfx::StencilOpType::Keep,
    /*.pass=*/gfx::StencilOpType::Replace,
};
const auto clipMaskDepthMode = gfx::DepthMode{/*.func=*/gfx::DepthFunctionType::Always,
                                              /*.mask=*/gfx::DepthMaskType::ReadOnly};
} // namespace

bool Context::renderTileClippingMasks(gfx::RenderPass& renderPass,
                                      RenderStaticData& staticData,
                                      const std::vector<shaders::ClipUBO>& tileUBOs) {
    using ShaderClass = shaders::ShaderSource<shaders::BuiltIn::ClippingMaskProgram, gfx::Backend::Type::Metal>;

    if (!clipMaskShader) {
        const auto group = staticData.shaders->getShaderGroup("ClippingMaskProgram");
        if (group) {
            clipMaskShader = std::static_pointer_cast<gfx::ShaderProgramBase>(group->getOrCreateShader(*this, {}));
        }
    }
    if (!clipMaskShader) {
        assert(!"Failed to create shader for clip masking");
        return false;
    }

    const auto& mtlShader = static_cast<const mtl::ShaderProgram&>(*clipMaskShader);
    auto& mtlRenderPass = static_cast<mtl::RenderPass&>(renderPass);
    const auto& encoder = mtlRenderPass.getMetalEncoder();
    const auto colorMode = gfx::ColorMode::disabled();

    // Create a vertex buffer from the fixed tile coordinates
    constexpr auto vertexSize = sizeof(gfx::Vertex<PositionOnlyLayoutAttributes>::a1);
    const auto& vertexRes = getTileVertexBuffer();
    if (!vertexRes) {
        assert(vertexSize == vertexRes.getSizeInBytes());
        return false;
    }

    // Create a buffer from the fixed tile indexes
    constexpr NS::UInteger indexCount = 6;
    const auto indexRes = &getTileIndexBuffer();
    if (!indexRes) {
        return false;
    }

    const auto& renderPassDescriptor = mtlRenderPass.getDescriptor();
    const auto& renderable = renderPassDescriptor.renderable;
    if (stencilStateRenderable != &renderable) {
        // We're on a new renderable, invalidate objects constructed for the previous one.
        clipMaskPipelineState.reset();
        clipMaskDepthStencilState.reset();
        stencilStateRenderable = &renderable;
    }

    // Create the depth-stencil state, if necessary.
    if (!clipMaskDepthStencilState) {
        if (auto depthStencilState = makeDepthStencilState(clipMaskDepthMode, clipMaskStencilMode, renderable)) {
            clipMaskDepthStencilState = std::move(depthStencilState);
        }
    }
    assert(clipMaskDepthStencilState || !"Failed to create depth-stencil state for clip masking");
    mtlRenderPass.setDepthStencilState(clipMaskDepthStencilState);

    if (!clipMaskPipelineState) {
        // A vertex descriptor tells Metal what's in the vertex buffer
        auto vertDesc = NS::RetainPtr(MTL::VertexDescriptor::vertexDescriptor());
        auto attribDesc = NS::TransferPtr(MTL::VertexAttributeDescriptor::alloc()->init());
        auto layoutDesc = NS::TransferPtr(MTL::VertexBufferLayoutDescriptor::alloc()->init());
        if (!vertDesc || !attribDesc || !layoutDesc) {
            return false;
        }

        attribDesc->setBufferIndex(ShaderClass::attributes[0].index);
        attribDesc->setOffset(0);
        attribDesc->setFormat(MTL::VertexFormatShort2);
        layoutDesc->setStride(static_cast<NS::UInteger>(vertexSize));
        layoutDesc->setStepFunction(MTL::VertexStepFunctionPerVertex);
        layoutDesc->setStepRate(1);
        vertDesc->attributes()->setObject(attribDesc.get(), ShaderClass::attributes[0].index);
        vertDesc->layouts()->setObject(layoutDesc.get(), ShaderClass::attributes[0].index);

        // Create a render pipeline state, telling Metal how to render the primitives
        const std::size_t hash = mbgl::util::hash(ShaderClass::attributes[0].index,
                                                  0,
                                                  MTL::VertexFormatShort2,
                                                  vertexSize,
                                                  MTL::VertexStepFunctionPerVertex,
                                                  1);
        if (auto state = mtlShader.getRenderPipelineState(
                renderable, vertDesc, colorMode, mbgl::util::hash(colorMode.hash(), hash))) {
            clipMaskPipelineState = std::move(state);
        }
    }
    if (clipMaskPipelineState) {
        encoder->setRenderPipelineState(clipMaskPipelineState.get());
    } else {
        assert(!"Failed to create render pipeline state for clip masking");
        return false;
    }

    // Create a buffer for the UBO data.
    constexpr auto uboSize = sizeof(shaders::ClipUBO);
    const auto bufferSize = tileUBOs.size() * uboSize;

    std::optional<BufferResource> tempBuffer;
    auto& uboBuffer = clipMaskUniformsBufferUsed ? tempBuffer : clipMaskUniformsBuffer;
    clipMaskUniformsBufferUsed = true;
    if (!uboBuffer || !*uboBuffer || uboBuffer->getSizeInBytes() < bufferSize) {
        uboBuffer = createBuffer(tileUBOs.data(),
                                 bufferSize,
                                 gfx::BufferUsageType::StaticDraw,
                                 /*isIndexBuffer=*/false,
                                 /*persistent=*/!clipMaskUniformsBufferUsed);
        if (!uboBuffer) {
            return false;
        }
    } else {
        uboBuffer->update(tileUBOs.data(), bufferSize, /*offset=*/0);
    }

    encoder->setCullMode(MTL::CullModeNone);

    mtlRenderPass.bindVertex(vertexRes, /*offset=*/0, ShaderClass::attributes[0].index);

    // Instancing is disabled for now because the `[[stencil]]` attribute in the fragment shader output
    // that we need to apply a different stencil value for each tile causes a problem on some older (A8-A11)
    // devices, specifically `XPC_ERROR_CONNECTION_INTERRUPTED` when creating a render pipeline state.
    // Adding a `[[depth(...)]]` output to the shader prevents this error, but the stencil value is
    // still not written to the stencil attachment on those same devices.
#if STENCIL_INSTANCING
    encoder->setVertexBuffer(uboBuffer.getMetalBuffer().get(), /*offset=*/0, shaders::idClippingMaskUBO);
    encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                   indexCount,
                                   MTL::IndexType::IndexTypeUInt16,
                                   indexRes->getMetalBuffer().get(),
                                   /*indexOffset=*/0,
                                   /*instanceCount=*/static_cast<NS::UInteger>(tileUBOs.size()));
#else
    for (std::size_t ii = 0; ii < tileUBOs.size(); ++ii) {
        encoder->setStencilReferenceValue(tileUBOs[ii].stencil_ref);
        mtlRenderPass.bindVertex(*uboBuffer, /*offset=*/ii * uboSize, shaders::idClippingMaskUBO, /*size=*/uboSize);
        encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                       indexCount,
                                       MTL::IndexType::IndexTypeUInt16,
                                       indexRes->getMetalBuffer().get(),
                                       /*indexOffset=*/0,
                                       /*instanceCount=*/1);
    }
#endif

    stats.numDrawCalls++;
    stats.totalDrawCalls++;
    return true;
}

void Context::setDirtyState() {}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size,
                                                                       gfx::TextureChannelDataType type,
                                                                       bool depth,
                                                                       bool stencil) {
    return std::make_unique<OffscreenTexture>(*this, size, type, depth, stencil);
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size, gfx::TextureChannelDataType type) {
    return createOffscreenTexture(size, type, false, false);
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType, Size) {
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    assert(false);
    return nullptr;
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return std::make_shared<VertexAttributeArray>();
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {}

void Context::visualizeDepthBuffer(float) {}
#endif // !defined(NDEBUG)

void Context::clearStencilBuffer(int32_t) {
    // See `PaintParameters::clearStencil`
    assert(false);
}

namespace {

MTL::CompareFunction mapFunc(const gfx::DepthFunctionType func) {
    switch (func) {
        default:
        case gfx::DepthFunctionType::Never:
            return MTL::CompareFunction::CompareFunctionNever;
        case gfx::DepthFunctionType::Less:
            return MTL::CompareFunction::CompareFunctionLess;
        case gfx::DepthFunctionType::Equal:
            return MTL::CompareFunction::CompareFunctionEqual;
        case gfx::DepthFunctionType::LessEqual:
            return MTL::CompareFunction::CompareFunctionLessEqual;
        case gfx::DepthFunctionType::Greater:
            return MTL::CompareFunction::CompareFunctionGreater;
        case gfx::DepthFunctionType::NotEqual:
            return MTL::CompareFunction::CompareFunctionNotEqual;
        case gfx::DepthFunctionType::GreaterEqual:
            return MTL::CompareFunction::CompareFunctionGreaterEqual;
        case gfx::DepthFunctionType::Always:
            return MTL::CompareFunction::CompareFunctionAlways;
    };
}
MTL::CompareFunction mapFunc(const gfx::StencilFunctionType func) {
    switch (func) {
        default:
        case gfx::StencilFunctionType::Never:
            return MTL::CompareFunction::CompareFunctionNever;
        case gfx::StencilFunctionType::Less:
            return MTL::CompareFunction::CompareFunctionLess;
        case gfx::StencilFunctionType::Equal:
            return MTL::CompareFunction::CompareFunctionEqual;
        case gfx::StencilFunctionType::LessEqual:
            return MTL::CompareFunction::CompareFunctionLessEqual;
        case gfx::StencilFunctionType::Greater:
            return MTL::CompareFunction::CompareFunctionGreater;
        case gfx::StencilFunctionType::NotEqual:
            return MTL::CompareFunction::CompareFunctionNotEqual;
        case gfx::StencilFunctionType::GreaterEqual:
            return MTL::CompareFunction::CompareFunctionGreaterEqual;
        case gfx::StencilFunctionType::Always:
            return MTL::CompareFunction::CompareFunctionAlways;
    };
}

MTL::StencilOperation mapOperation(const gfx::StencilOpType op) {
    switch (op) {
        case gfx::StencilOpType::Zero:
            return MTL::StencilOperation::StencilOperationZero;
        default:
        case gfx::StencilOpType::Keep:
            return MTL::StencilOperation::StencilOperationKeep;
        case gfx::StencilOpType::Replace:
            return MTL::StencilOperation::StencilOperationReplace;
        case gfx::StencilOpType::Increment:
            return MTL::StencilOperation::StencilOperationIncrementClamp;
        case gfx::StencilOpType::Decrement:
            return MTL::StencilOperation::StencilOperationDecrementClamp;
        case gfx::StencilOpType::Invert:
            return MTL::StencilOperation::StencilOperationInvert;
        case gfx::StencilOpType::IncrementWrap:
            return MTL::StencilOperation::StencilOperationIncrementWrap;
        case gfx::StencilOpType::DecrementWrap:
            return MTL::StencilOperation::StencilOperationDecrementWrap;
    }
}

void applyDepthMode(const gfx::DepthMode& depthMode, MTL::DepthStencilDescriptor* desc) {
    desc->setDepthCompareFunction(mapFunc(depthMode.func));
    desc->setDepthWriteEnabled(depthMode.mask == gfx::DepthMaskType::ReadWrite);
    // depthMode.range ?
}

struct StencilModeVisitor {
    MTL::StencilDescriptor* desc;

    template <gfx::StencilFunctionType F>
    void operator()(const gfx::StencilMode::SimpleTest<F>& mode) {
        apply(mode.func, std::numeric_limits<uint32_t>::max());
    }
    template <gfx::StencilFunctionType F>
    void operator()(const gfx::StencilMode::MaskedTest<F>& mode) {
        apply(mode.func, mode.mask);
    }

    void apply(gfx::StencilFunctionType func, uint32_t mask) {
        desc->setStencilCompareFunction(mapFunc(func));
        desc->setReadMask(mask);
        desc->setWriteMask(mask);
    }
};

// helper type for the visitor #4
void applyStencilMode(const gfx::StencilMode& stencilMode, MTL::StencilDescriptor* desc) {
    mapbox::util::apply_visitor(StencilModeVisitor{desc}, stencilMode.test);

    desc->setStencilFailureOperation(mapOperation(stencilMode.fail));
    desc->setDepthFailureOperation(mapOperation(stencilMode.depthFail));
    desc->setDepthStencilPassOperation(mapOperation(stencilMode.pass));
}

} // namespace

MTLDepthStencilStatePtr Context::makeDepthStencilState(const gfx::DepthMode& depthMode,
                                                       const gfx::StencilMode& stencilMode,
                                                       const gfx::Renderable& renderable) const {
    auto depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    if (!depthStencilDescriptor) {
        return {};
    }

    auto& device = backend.getDevice();
    const auto& renderableResource = renderable.getResource<RenderableResource>();
    if (const auto& rpd = renderableResource.getRenderPassDescriptor()) {
        // Setting depth/stencil properties when the corresponding target textures aren't set causes, e.g.:
        // `Draw Errors Validation MTLDepthStencilDescriptor sets depth test but MTLRenderPassDescriptor has a nil
        // depthAttachment texture`
        if (auto* depthTarget = rpd->depthAttachment()) {
            if (depthTarget->texture()) {
                applyDepthMode(depthMode, depthStencilDescriptor.get());
            }
        }
        if (auto* stencilTarget = rpd->stencilAttachment()) {
            if (stencilTarget->texture()) {
                auto stencilDescriptor = NS::TransferPtr(MTL::StencilDescriptor::alloc()->init());
                if (!stencilDescriptor) {
                    return {};
                }

                applyStencilMode(stencilMode, stencilDescriptor.get());

                depthStencilDescriptor->setFrontFaceStencil(stencilDescriptor.get());
                depthStencilDescriptor->setBackFaceStencil(stencilDescriptor.get());
            }
        }
    }

    return NS::TransferPtr(device->newDepthStencilState(depthStencilDescriptor.get()));
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass& renderPass) const noexcept {
    auto& mtlRenderPass = static_cast<mtl::RenderPass&>(renderPass);
    globalUniformBuffers.bindMtl(mtlRenderPass);
}

} // namespace mtl
} // namespace mbgl
