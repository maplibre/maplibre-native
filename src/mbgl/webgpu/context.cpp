#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/context_impl.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/drawable_builder.hpp>
#include <mbgl/webgpu/draw_scope_resource.hpp>
#include <mbgl/webgpu/offscreen_texture.hpp>
#include <mbgl/webgpu/renderbuffer.hpp>
#include <mbgl/webgpu/renderbuffer_resource.hpp>
#include <mbgl/webgpu/uniform_buffer.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/shaders/shader_source.hpp>
// Include all WebGPU shader headers
#include <mbgl/shaders/webgpu/background.hpp>
#include <mbgl/shaders/webgpu/circle.hpp>
#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/webgpu/collision.hpp>
#include <mbgl/shaders/webgpu/custom_geometry.hpp>
#include <mbgl/shaders/webgpu/custom_symbol_icon.hpp>
#include <mbgl/shaders/webgpu/debug.hpp>
#include <mbgl/shaders/webgpu/fill.hpp>
#include <mbgl/shaders/webgpu/fill_extrusion.hpp>
#include <mbgl/shaders/webgpu/heatmap.hpp>
#include <mbgl/shaders/webgpu/heatmap_texture.hpp>
#include <mbgl/shaders/webgpu/hillshade.hpp>
#include <mbgl/shaders/webgpu/hillshade_prepare.hpp>
#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/webgpu/raster.hpp>
#include <mbgl/shaders/webgpu/symbol.hpp>
#include <mbgl/webgpu/texture2d.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/webgpu/tile_layer_group.hpp>
#include <mbgl/webgpu/layer_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

Context::Context(RendererBackend& backend_)
    : gfx::Context(gfx::Context::minimumRequiredVertexBindingCount),
      impl(std::make_unique<Impl>()),
      backend(backend_),
      globalUniformBuffers(std::make_unique<UniformBufferArray>()) {

    impl->device = backend.getDevice();

}

Context::~Context() = default;

void Context::beginFrame() {
    // Begin a new frame - WebGPU command recording starts here
}

void Context::endFrame() {
    // End the frame - submit WebGPU commands
}

void Context::performCleanup() {
    // Clean up unused resources
}

void Context::reduceMemoryUsage() {
    // Free cached resources to reduce memory
}

std::unique_ptr<gfx::OffscreenTexture> Context::createOffscreenTexture(Size size, gfx::TextureChannelDataType type) {
    return std::make_unique<OffscreenTexture>(*this, size, type, true, false);
}

std::unique_ptr<gfx::CommandEncoder> Context::createCommandEncoder() {
    return std::make_unique<CommandEncoder>(*this);
}

gfx::VertexAttributeArrayPtr Context::createVertexAttributeArray() const {
    return std::make_shared<VertexAttributeArray>();
}

gfx::UniqueDrawableBuilder Context::createDrawableBuilder(std::string name) {
    return std::make_unique<DrawableBuilder>(std::move(name));
}

gfx::UniformBufferPtr Context::createUniformBuffer(const void* data, std::size_t size, bool persistent, bool) {
    (void)persistent;
    return std::make_shared<UniformBuffer>(*this, data, size);
}

gfx::UniqueUniformBufferArray Context::createLayerUniformBufferArray() {
    return std::make_unique<UniformBufferArray>();
}

gfx::ShaderProgramBasePtr Context::getGenericShader(gfx::ShaderRegistry& registry, const std::string& name) {
    auto it = impl->shaderCache.find(name);
    if (it != impl->shaderCache.end()) {
        return it->second;
    }

    // Get the shader group from registry
    auto& shaderGroup = registry.getShaderGroup(name);
    if (!shaderGroup) {
        // If no shader group exists, create and initialize one
        auto webgpuShaderGroup = std::make_shared<webgpu::ShaderGroup>();
        webgpuShaderGroup->initialize(*this);
        bool registered = registry.registerShaderGroup(std::move(webgpuShaderGroup), name);
        (void)registered; // Ignore return value
    }

    // Try to get the shader from the group
    if (shaderGroup) {
        auto shader = shaderGroup->getShader(name);
        if (shader) {
            // Convert gfx::Shader to ShaderProgramBase
            // Since our ShaderProgram inherits from ShaderProgramBase, we need to cast
            auto shaderProgram = std::static_pointer_cast<gfx::ShaderProgramBase>(shader);
            impl->shaderCache[name] = shaderProgram;
            return shaderProgram;
        }
    }

    // Fallback: Use the built-in WGSL shaders based on name
    std::string vertexSource;
    std::string fragmentSource;

    using namespace shaders;

    // Map shader names to built-in shaders
#define MAP_SHADER(ShaderType, ShaderName) \
    if (name == ShaderName) { \
        vertexSource = ShaderSource<BuiltIn::ShaderType, gfx::Backend::Type::WebGPU>::vertex; \
        fragmentSource = ShaderSource<BuiltIn::ShaderType, gfx::Backend::Type::WebGPU>::fragment; \
    } else

    MAP_SHADER(BackgroundShader, "BackgroundShader")
    MAP_SHADER(BackgroundPatternShader, "BackgroundPatternShader")
    MAP_SHADER(CircleShader, "CircleShader")
    MAP_SHADER(ClippingMaskProgram, "ClippingMaskProgram")
    MAP_SHADER(CollisionBoxShader, "CollisionBoxShader")
    MAP_SHADER(CollisionCircleShader, "CollisionCircleShader")
    MAP_SHADER(CustomGeometryShader, "CustomGeometryShader")
    MAP_SHADER(CustomSymbolIconShader, "CustomSymbolIconShader")
    MAP_SHADER(DebugShader, "DebugShader")
    MAP_SHADER(FillShader, "FillShader")
    MAP_SHADER(FillOutlineShader, "FillOutlineShader")
    MAP_SHADER(FillExtrusionShader, "FillExtrusionShader")
    MAP_SHADER(FillExtrusionPatternShader, "FillExtrusionPatternShader")
    MAP_SHADER(HeatmapShader, "HeatmapShader")
    MAP_SHADER(HeatmapTextureShader, "HeatmapTextureShader")
    MAP_SHADER(HillshadePrepareShader, "HillshadePrepareShader")
    MAP_SHADER(HillshadeShader, "HillshadeShader")
    MAP_SHADER(LineShader, "LineShader")
    MAP_SHADER(RasterShader, "RasterShader")
    MAP_SHADER(SymbolIconShader, "SymbolIconShader")
    MAP_SHADER(SymbolSDFShader, "SymbolSDFShader")
    {
        // For now, use a basic WGSL shader that works for all types
        // This is a temporary solution until proper shaders are implemented
        vertexSource = R"(
@vertex
fn main(
    @builtin(vertex_index) vertex_index: u32
) -> @builtin(position) vec4<f32> {
    // Generate a hardcoded triangle in NDC space for testing
    // This doesn't use any vertex buffers or uniforms
    var positions = array<vec2<f32>, 3>(
        vec2<f32>(-0.8, -0.8),  // Bottom left
        vec2<f32>(0.8, -0.8),   // Bottom right
        vec2<f32>(0.0, 0.8)     // Top center
    );
    return vec4<f32>(positions[vertex_index], 0.0, 1.0);
}
)";

        fragmentSource = R"(
@fragment
fn main() -> @location(0) vec4<f32> {
    // Return a bright red color for debugging visibility
    return vec4<f32>(1.0, 0.0, 0.0, 1.0); // Bright red
}
)";

    }

#undef MAP_SHADER

    // Create new shader program
    auto shader = std::make_shared<ShaderProgram>(*this, vertexSource, fragmentSource);
    impl->shaderCache[name] = shader;

    return shader;
}

TileLayerGroupPtr Context::createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    auto tileLayerGroup = std::make_shared<webgpu::TileLayerGroup>(layerIndex, initialCapacity, std::move(name));
    return tileLayerGroup;
}

LayerGroupPtr Context::createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) {
    auto layerGroup = std::make_shared<webgpu::LayerGroup>(layerIndex, initialCapacity, std::move(name));
    return layerGroup;
}

gfx::Texture2DPtr Context::createTexture2D() {
    return std::make_shared<Texture2D>(*this);
}

RenderTargetPtr Context::createRenderTarget(const Size size, const gfx::TextureChannelDataType type) {
    return std::make_shared<RenderTarget>(*this, size, type);
}

void Context::resetState(gfx::DepthMode, gfx::ColorMode) {
    // Reset WebGPU render state
}

void Context::setDirtyState() {
    // Mark state as needing update
}

void Context::clearStencilBuffer(int32_t value) {
    // Clear stencil buffer to specified value
    (void)value;
}

bool Context::emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr& ptr, const void* data, std::size_t size, bool persistent) {
    if (!ptr) {
        ptr = createUniformBuffer(data, size, persistent);
        return true;
    }

    // Update existing buffer
    auto* buffer = static_cast<UniformBuffer*>(ptr.get());
    buffer->update(data, size);
    return false;
}

const gfx::UniformBufferArray& Context::getGlobalUniformBuffers() const {
    return *globalUniformBuffers;
}

gfx::UniformBufferArray& Context::mutableGlobalUniformBuffers() {
    return *globalUniformBuffers;
}

void Context::bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    // Bind global uniform buffers for rendering
}

void Context::unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept {
    // Unbind global uniform buffers
}

#if !defined(NDEBUG)
void Context::visualizeStencilBuffer() {
    // Debug visualization of stencil buffer
}

void Context::visualizeDepthBuffer([[maybe_unused]] float depthRangeSize) {
    // Debug visualization of depth buffer
}
#endif

std::unique_ptr<gfx::RenderbufferResource> Context::createRenderbufferResource(gfx::RenderbufferPixelType type, Size size) {
    (void)type;
    (void)size;
    return std::make_unique<RenderbufferResource>();
}

std::unique_ptr<gfx::DrawScopeResource> Context::createDrawScopeResource() {
    return std::make_unique<DrawScopeResource>(*this);
}

} // namespace webgpu
} // namespace mbgl
