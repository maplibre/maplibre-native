#include <mbgl/mtl/context.hpp>

#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/layer_group.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/tile_layer_group.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

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
    return std::make_unique<mtl::CommandEncoder>(*this);
}

UniqueShaderProgram Context::createProgram(std::string name,
                                           const std::string_view source,
                                           const std::string_view vertexName,
                                           const std::string_view fragmentName,
                                           const ProgramParameters& programParameters,
                                           const std::unordered_map<std::string, std::string>& additionalDefines)
{
    auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    auto device = backend.getDevice();

    auto defines = NS::Dictionary::alloc()->init();

    //auto options = NS::TransferPtr(MTL::CompileOptions::alloc()->init());
    auto options = MTL::CompileOptions::alloc()->init();

    options->setPreprocessorMacros(defines);
    options->setFastMathEnabled(true);
    options->setLanguageVersion(MTL::LanguageVersion3_0);
    options->setLibraryType(MTL::LibraryTypeExecutable);
    //options->setInstallName(const NS::String* installName);
    //options->setLibraries(const NS::Array* libraries);
    options->setPreserveInvariance(true);
    options->setOptimizationLevel(MTL::LibraryOptimizationLevelDefault);
    options->setCompileSymbolVisibility(MTL::CompileSymbolVisibilityDefault);
    options->setAllowReferencingUndefinedSymbols(false);
    //options->setMaxTotalThreadsPerThreadgroup(NS::UInteger);

    NS::Error* error = nullptr;
    NS::String* nsSource = NS::String::string(source.data(), NS::UTF8StringEncoding);

    MTL::Library* library = device->newLibrary(nsSource, nullptr, &error);
    if (!library)
    {
        const auto errPtr = error ? error->localizedDescription()->utf8String() : nullptr;
        if (errPtr && errPtr[0]) {
            Log::Error(Event::Shader, name + " compile failed: " + std::string(errPtr));
        } else {
            Log::Error(Event::Shader, name + " compile failed");
        }
        assert(false);
        return nullptr;
    }

    const auto nsVertName = NS::String::string(vertexName.data(), NS::UTF8StringEncoding);
    NS::SharedPtr<MTL::Function> vertexFunction = NS::TransferPtr(library->newFunction(nsVertName));
    if (!vertexFunction) {
        Log::Error(Event::Shader, name + " missing vertex function " + vertexName.data());
        assert(false);
        return nullptr;
    }

    // fragment function is optional
    NS::SharedPtr<MTL::Function> fragmentFunction;
    if (!fragmentName.empty()) {
        const auto nsFragName = NS::String::string(fragmentName.data(), NS::UTF8StringEncoding);
        fragmentFunction = NS::TransferPtr(library->newFunction(nsFragName));
        if (!fragmentFunction) {
            Log::Error(Event::Shader, name + " missing fragment function " + fragmentName.data());
            assert(false);
            return nullptr;
        }
    }
    
    return std::make_unique<ShaderProgram>(std::move(name), std::move(vertexFunction), std::move(fragmentFunction));
}

void Context::performCleanup() {
}

void Context::reduceMemoryUsage() {
}

#if MLN_DRAWABLE_RENDERER
gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    assert(false);
    return nullptr;
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size) {
    assert(false);
    return nullptr;
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& shaders, const std::string& name) {
    std::vector<std::string> emptyProperties(0);
    return std::static_pointer_cast<gfx::ShaderProgramBase>(
        shaders.getShaderGroup(name)->getOrCreateShader(*this, emptyProperties));
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

void Context::resetState(gfx::DepthMode depthMode, gfx::ColorMode colorMode) {
}

#endif // MLN_DRAWABLE_RENDERER

void Context::setDirtyState() {
}

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

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                   Size size) {
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    assert(false);
    return nullptr;
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
}

void Context::visualizeDepthBuffer(float depthRangeSize) {
}
#endif // !defined(NDEBUG)

void Context::clearStencilBuffer(int32_t) {
}

} // namespace mtl
} // namespace mbgl

