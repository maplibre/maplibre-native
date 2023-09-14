#include <mbgl/mtl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/drawable_builder.hpp>
#include <mbgl/mtl/layer_group.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/tile_layer_group.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/programs/program_parameters.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>

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
      backend(backend_),
      stats() {}

Context::~Context() noexcept {
    if (cleanupOnDestruction) {
        performCleanup();
        assert(stats.isZero());
    }
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<CommandEncoder>(*this);
}

BufferResource Context::createBuffer(const void* data, std::size_t size, gfx::BufferUsageType) const {
    return {backend.getDevice(), data, size, MTL::ResourceStorageModeShared};
}

UniqueShaderProgram Context::createProgram(std::string name,
                                           const std::string_view source,
                                           const std::string_view vertexName,
                                           const std::string_view fragmentName,
                                           const ProgramParameters& programParameters,
                                           const std::unordered_map<std::string, std::string>& additionalDefines) {
    const auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    // No NSMutableDictionary?
    const auto& programDefines = programParameters.getDefines();
    const auto numDefines = programDefines.size() + additionalDefines.size();

    std::vector<NS::Object*> rawDefines;
    rawDefines.reserve(2 * numDefines);
    const auto addDefine = [&rawDefines](const auto& pair) {
        auto* nsKey = NS::String::string(pair.first.data(), NS::UTF8StringEncoding);
        auto* nsVal = NS::String::string(pair.second.data(), NS::UTF8StringEncoding);
        rawDefines.insert(std::next(rawDefines.begin(), rawDefines.size() / 2), nsKey);
        rawDefines.insert(rawDefines.end(), nsVal);
    };
    std::for_each(programDefines.begin(), programDefines.end(), addDefine);
    std::for_each(additionalDefines.begin(), additionalDefines.end(), addDefine);

    const auto nsDefines = NS::Dictionary::dictionary(
        &rawDefines[numDefines], rawDefines.data(), static_cast<NS::UInteger>(numDefines));
    rawDefines.clear();

    auto options = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    options->setPreprocessorMacros(nsDefines);
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

    const auto& device = backend.getDevice();
    MTL::Library* library = device->newLibrary(nsSource, options.get(), &error);
    if (!library || error) {
        const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
        const auto errStr = (errPtr && errPtr[0]) ? ": " + std::string(errPtr) : std::string();
        Log::Error(Event::Shader, name + " compile failed" + errStr);
        assert(false);
        return nullptr;
    }

    const auto nsVertName = NS::String::string(vertexName.data(), NS::UTF8StringEncoding);
    NS::SharedPtr<MTL::Function> vertexFunction = NS::RetainPtr(library->newFunction(nsVertName));
    if (!vertexFunction) {
        Log::Error(Event::Shader, name + " missing vertex function " + vertexName.data());
        assert(false);
        return nullptr;
    }

    // fragment function is optional
    NS::SharedPtr<MTL::Function> fragmentFunction;
    if (!fragmentName.empty()) {
        const auto nsFragName = NS::String::string(fragmentName.data(), NS::UTF8StringEncoding);
        fragmentFunction = NS::RetainPtr(library->newFunction(nsFragName));
        if (!fragmentFunction) {
            Log::Error(Event::Shader, name + " missing fragment function " + fragmentName.data());
            assert(false);
            return nullptr;
        }
    }

    return std::make_unique<ShaderProgram>(
        std::move(name), backend, std::move(vertexFunction), std::move(fragmentFunction));
}

MTLTexturePtr Context::createMetalTexture(MTLTextureDescriptorPtr textureDescriptor) const {
    return NS::TransferPtr(backend.getDevice()->newTexture(textureDescriptor.get()));
}

MTLSamplerStatePtr Context::createMetalSamplerState(MTLSamplerDescriptorPtr samplerDescriptor) const {
    return NS::TransferPtr(backend.getDevice()->newSamplerState(samplerDescriptor.get()));
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size) {
    return std::make_shared<UniformBuffer>(createBuffer(data, size, gfx::BufferUsageType::StaticDraw));
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    std::vector<std::string> emptyProperties(0);
    auto shaderGroup = shaders.getShaderGroup(name);
    if (!shaderGroup) {
        return nullptr;
    }
    return std::static_pointer_cast<gfx::ShaderProgramBase>(shaderGroup->getOrCreateShader(*this, emptyProperties));
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
    assert(false);
    return nullptr;
}

void Context::resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) {}

void Context::setDirtyState() {}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size, gfx::TextureChannelDataType) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::TextureResource> Context::createTextureResource(Size,
                                                                     gfx::TexturePixelType,
                                                                     gfx::TextureChannelDataType) {
    assert(false);
    return nullptr;
}

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType, Size size) {
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    assert(false);
    return nullptr;
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {}

void Context::visualizeDepthBuffer(float depthRangeSize) {}
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
                                                       const mtl::RenderPass& renderPass) const {
    auto depthStencilDescriptor = NS::TransferPtr(MTL::DepthStencilDescriptor::alloc()->init());
    if (!depthStencilDescriptor) {
        return {};
    }

    auto& device = backend.getDevice();
    const auto& renderPassDescriptor = renderPass.getDescriptor();
    const auto& renderable = renderPassDescriptor.renderable;
    const auto& renderableResource = renderable.getResource<RenderableResource>();
    if (const auto& rpd = renderableResource.getRenderPassDescriptor()) {
        // Setting depth/stencil properties when the corresponding target textures aren't set causes, e.g.:
        // `Draw Errors Validation MTLDepthStencilDescriptor sets depth test but MTLRenderPassDescriptor has a nil
        // depthAttachment texture`
        if (auto* depthTarget = rpd->depthAttachment()) {
            if (auto* tex = depthTarget->texture()) {
                applyDepthMode(depthMode, depthStencilDescriptor.get());
            }
        }
        if (auto* stencilTarget = rpd->stencilAttachment()) {
            if (auto* tex = stencilTarget->texture()) {
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

} // namespace mtl
} // namespace mbgl
