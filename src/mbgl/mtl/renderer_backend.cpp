#include <mbgl/mtl/renderer_backend.hpp>

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/mtl/shader_group.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/util/logging.hpp>

#include <cassert>
#include <string>

namespace mbgl {
namespace shaders {

template <>
struct ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundShader";
    static constexpr auto vertexMainFunction = "vertexMain";
    static constexpr auto fragmentMainFunction = "fragmentMain";

    static constexpr AttributeInfo attributes[] = {
        {0, "a_pos", gfx::AttributeDataType::Float3, 1},
    };
    static constexpr UniformBlockInfo uniforms[] = {
        {1, sizeof(BackgroundLayerUBO), true, true, "BackgroundLayerUBO"},
        {2, sizeof(BackgroundDrawableUBO), true, false, "BackgroundDrawableUBO"},
    };

    static constexpr auto source = R"(
#include <metal_stdlib>
using namespace metal;

struct alignas(16) BackgroundDrawableUBO {
    float4x4 matrix;
};
struct alignas(16) BackgroundLayerUBO {
    float4 color;
    float opacity, pad1, pad2, pad3;
};

struct v2f {
    float4 position [[position]];
};

v2f vertex vertexMain(uint vertexId [[vertex_id]],
                      device const short2* positions [[buffer(0)]],
                      device const BackgroundLayerUBO& layerUBO [[buffer(1)]],
                      device const BackgroundDrawableUBO& drawableUBO [[buffer(2)]]) {
    return { drawableUBO.matrix * float4(positions[vertexId].x, positions[vertexId].y, 0.0, 1.0) };
}

half4 fragment fragmentMain(v2f in [[stage_in]],
                            device const BackgroundLayerUBO& layerUBO [[buffer(1)]]) {
#ifdef OVERDRAW_INSPECTOR
    return half4(1.0);
#else
    return half4(layerUBO.color) * layerUBO.opacity;
#endif
}
)";
};

template <>
struct ShaderSource<BuiltIn::BackgroundPatternShader, gfx::Backend::Type::Metal>
    : public ShaderSource<BuiltIn::BackgroundShader, gfx::Backend::Type::Metal> {
    static constexpr auto name = "BackgroundPatternShader";
    static constexpr UniformBlockInfo uniforms[] = {
        {1, sizeof(BackgroundLayerUBO), true, false, "BackgroundLayerUBO"},
        {2, sizeof(BackgroundDrawableUBO), true, false, "BackgroundDrawableUBO"},
        {3, sizeof(BackgroundPatternLayerUBO), true, false, "BackgroundPatternLayerUBO"},
    };
};

} // namespace shaders
} // namespace mbgl

namespace mbgl {
namespace mtl {

RendererBackend::RendererBackend(const gfx::ContextMode contextMode_)
    : gfx::RendererBackend(contextMode_),
      device(NS::TransferPtr(MTL::CreateSystemDefaultDevice())),
      commandQueue(NS::TransferPtr(device->newCommandQueue())) {
    assert(device);
    assert(commandQueue);
}

RendererBackend::~RendererBackend() = default;

std::unique_ptr<gfx::Context> RendererBackend::createContext() {
    return std::make_unique<mtl::Context>(*this);
}

PremultipliedImage RendererBackend::readFramebuffer(const Size& size) {
    return PremultipliedImage(size);
    // return getContext<mtl::Context>().readFramebuffer<PremultipliedImage>(size);
}

void RendererBackend::assumeFramebufferBinding(const mtl::FramebufferID fbo) {
    /*getContext<mtl::Context>().bindFramebuffer.setCurrentValue(fbo);
    if (fbo != ImplicitFramebufferBinding) {
        assert(mtl::value::BindFramebuffer::Get() == getContext<mtl::Context>().bindFramebuffer.getCurrentValue());
    }*/
}

void RendererBackend::assumeViewport(int32_t x, int32_t y, const Size& size) {
    /*getContext<mtl::Context>().viewport.setCurrentValue({x, y, size});
    assert(mtl::value::Viewport::Get() == getContext<mtl::Context>().viewport.getCurrentValue());*/
}

void RendererBackend::assumeScissorTest(bool enabled) {
    /*getContext<mtl::Context>().scissorTest.setCurrentValue(enabled);
    assert(mtl::value::ScissorTest::Get() == getContext<mtl::Context>().scissorTest.getCurrentValue());*/
}

bool RendererBackend::implicitFramebufferBound() {
    return false;
    // return getContext<mtl::Context>().bindFramebuffer.getCurrentValue() == ImplicitFramebufferBinding;
}

void RendererBackend::setFramebufferBinding(const mtl::FramebufferID fbo) {
    /*getContext<mtl::Context>().bindFramebuffer = fbo;
    if (fbo != ImplicitFramebufferBinding) {
        assert(mtl::value::BindFramebuffer::Get() == getContext<mtl::Context>().bindFramebuffer.getCurrentValue());
    }*/
}

void RendererBackend::setViewport(int32_t x, int32_t y, const Size& size) {
    /*getContext<mtl::Context>().viewport = {x, y, size};
    assert(mtl::value::Viewport::Get() == getContext<mtl::Context>().viewport.getCurrentValue());*/
}

void RendererBackend::setScissorTest(bool enabled) {
    /*getContext<mtl::Context>().scissorTest = enabled;
    assert(mtl::value::ScissorTest::Get() == getContext<mtl::Context>().scissorTest.getCurrentValue());*/
}

/// @brief Register a list of types with a shader registry instance
/// @tparam ...ShaderID Pack of BuiltIn:: shader IDs
/// @param registry A shader registry instance
/// @param programParameters ProgramParameters used to initialize each instance
template <shaders::BuiltIn... ShaderID>
void registerTypes(gfx::ShaderRegistry& registry, const ProgramParameters& programParameters) {
    /// The following fold expression will create a shader for every type
    /// in the parameter pack and register it with the shader registry.

    /// Registration calls are wrapped in a lambda that throws on registration
    /// failure, we shouldn't expect registration to faill unless the shader
    /// registry instance provided already has conflicting programs present.
    (
        [&]() {
            using namespace std::string_literals;
            using ShaderClass = shaders::ShaderSource<ShaderID, gfx::Backend::Type::Metal>;
            auto group = std::make_shared<ShaderGroup<ShaderID>>(programParameters);
            if (!registry.registerShaderGroup(std::move(group), ShaderClass::name)) {
                assert(!"duplicate shader group");
                throw std::runtime_error("Failed to register "s + ShaderClass::name + " with shader registry!");
            }
        }(),
        ...);
}

void RendererBackend::initShaders(gfx::ShaderRegistry& shaders, const ProgramParameters& programParameters) {
    registerTypes<shaders::BuiltIn::BackgroundShader,
                  shaders::BuiltIn::BackgroundPatternShader/*,
                  shaders::BuiltIn::CircleShader,
                  shaders::BuiltIn::FillShader,
                  shaders::BuiltIn::FillOutlineShader,
                  shaders::BuiltIn::LineShader,
                  shaders::BuiltIn::LineSDFShader,
                  shaders::BuiltIn::LinePatternShader,
                  shaders::BuiltIn::LineGradientShader,
                  shaders::BuiltIn::FillOutlinePatternShader,
                  shaders::BuiltIn::FillPatternShader,
                  shaders::BuiltIn::FillExtrusionShader,
                  shaders::BuiltIn::FillExtrusionPatternShader,
                  shaders::BuiltIn::HeatmapShader,
                  shaders::BuiltIn::HeatmapTextureShader,
                  shaders::BuiltIn::HillshadePrepareShader,
                  shaders::BuiltIn::HillshadeShader,
                  shaders::BuiltIn::RasterShader,
                  shaders::BuiltIn::SymbolIconShader,
                  shaders::BuiltIn::SymbolSDFTextShader,
                  shaders::BuiltIn::SymbolSDFIconShader,
                  shaders::BuiltIn::SymbolTextAndIconShader*/>(shaders, programParameters);
}

} // namespace mtl
} // namespace mbgl
