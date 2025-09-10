#include <mbgl/shaders/webgpu/shader_group.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/shaders/webgpu/shader_program.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/logging.hpp>

// Include all WGSL shader headers
#include <mbgl/shaders/webgpu/background.hpp>
#include <mbgl/shaders/webgpu/circle.hpp>
#include <mbgl/shaders/webgpu/clipping_mask.hpp>
#include <mbgl/shaders/webgpu/collision.hpp>
#include <mbgl/shaders/webgpu/custom_geometry.hpp>
#include <mbgl/shaders/webgpu/debug.hpp>
#include <mbgl/shaders/webgpu/fill.hpp>
#include <mbgl/shaders/webgpu/fill_extrusion.hpp>
#include <mbgl/shaders/webgpu/heatmap.hpp>
#include <mbgl/shaders/webgpu/hillshade.hpp>
#include <mbgl/shaders/webgpu/line.hpp>
#include <mbgl/shaders/webgpu/raster.hpp>
#include <mbgl/shaders/webgpu/symbol.hpp>

namespace mbgl {
namespace webgpu {

void ShaderGroup::initialize(Context& context) {
    using namespace shaders;
    
    // Helper macro to register shaders
#define REGISTER_SHADER(ShaderType, ShaderName) \
    { \
        auto vertexSource = ShaderSource<BuiltIn::ShaderType, gfx::Backend::Type::WebGPU>::vertex; \
        auto fragmentSource = ShaderSource<BuiltIn::ShaderType, gfx::Backend::Type::WebGPU>::fragment; \
        auto shader = std::make_shared<ShaderProgram>(context, vertexSource, fragmentSource); \
        registerShader(std::move(shader), ShaderName); \
    }
    
    // Register all shaders
    REGISTER_SHADER(BackgroundShader, "BackgroundShader")
    REGISTER_SHADER(BackgroundPatternShader, "BackgroundPatternShader")
    REGISTER_SHADER(CircleShader, "CircleShader")
    REGISTER_SHADER(ClippingMaskProgram, "ClippingMaskProgram")
    REGISTER_SHADER(CollisionBoxShader, "CollisionBoxShader")
    REGISTER_SHADER(CollisionCircleShader, "CollisionCircleShader")
    REGISTER_SHADER(CustomGeometryShader, "CustomGeometryShader")
    REGISTER_SHADER(CustomSymbolIconShader, "CustomSymbolIconShader")
    REGISTER_SHADER(DebugShader, "DebugShader")
    REGISTER_SHADER(FillShader, "FillShader")
    REGISTER_SHADER(FillOutlineShader, "FillOutlineShader")
    REGISTER_SHADER(FillExtrusionShader, "FillExtrusionShader")
    REGISTER_SHADER(FillExtrusionPatternShader, "FillExtrusionPatternShader")
    REGISTER_SHADER(HeatmapShader, "HeatmapShader")
    REGISTER_SHADER(HeatmapTextureShader, "HeatmapTextureShader")
    REGISTER_SHADER(HillshadePrepareShader, "HillshadePrepareShader")
    REGISTER_SHADER(HillshadeShader, "HillshadeShader")
    REGISTER_SHADER(LineShader, "LineShader")
    REGISTER_SHADER(RasterShader, "RasterShader")
    REGISTER_SHADER(SymbolIconShader, "SymbolIconShader")
    REGISTER_SHADER(SymbolSDFShader, "SymbolSDFShader")
    
#undef REGISTER_SHADER
    
    Log::Info(Event::General, "WebGPU shaders initialized");
}

gfx::ShaderPtr ShaderGroup::getOrCreateShader(gfx::Context& context,
                                              const StringIDSetsPair& propertiesAsUniforms,
                                              std::string_view firstAttribName) {
    // For now, return nullptr for data-driven shaders
    // In a complete implementation, this would generate shader variants
    // based on which properties are data-driven
    (void)context;
    (void)propertiesAsUniforms;
    (void)firstAttribName;
    
    return nullptr;
}

} // namespace webgpu
} // namespace mbgl