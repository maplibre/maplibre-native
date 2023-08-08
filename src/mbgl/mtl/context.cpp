#include <mbgl/mtl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/drawable_builder.hpp>
#include <mbgl/mtl/layer_group.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/tile_layer_group.hpp>
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

Context::Context(RendererBackend& backend_)
    : gfx::Context(16), // TODO
      backend(backend_),
      stats() {}

Context::~Context() noexcept {
    /*
        if (cleanupOnDestruction) {
            reset();
            assert(stats.isZero());
        }
     */
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

    auto options = MTL::CompileOptions::alloc()->init();
    options->setPreprocessorMacros(nsDefines);
    options->setFastMathEnabled(true);
    options->setLanguageVersion(MTL::LanguageVersion3_0);
    options->setLibraryType(MTL::LibraryTypeExecutable);
    options->setPreserveInvariance(true);
    options->setOptimizationLevel(MTL::LibraryOptimizationLevelDefault);
    options->setCompileSymbolVisibility(MTL::CompileSymbolVisibilityDefault);
    options->setAllowReferencingUndefinedSymbols(false);
    // options->setMaxTotalThreadsPerThreadgroup(NS::UInteger);

    NS::Error* error = nullptr;
    NS::String* nsSource = NS::String::string(source.data(), NS::UTF8StringEncoding);

    const auto& device = backend.getDevice();
    MTL::Library* library = device->newLibrary(nsSource, nullptr, &error);
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

void Context::performCleanup() {}

void Context::reduceMemoryUsage() {}

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

void Context::clearStencilBuffer(int32_t) {}

} // namespace mtl
} // namespace mbgl
