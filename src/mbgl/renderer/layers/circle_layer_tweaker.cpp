#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/circle.hpp>
#endif

#include <cstring>

namespace mbgl {

using namespace style;
using namespace shaders;

#if MLN_RENDER_BACKEND_METAL
void CircleLayerTweaker::setPropertiesAsUniforms([[maybe_unused]] std::vector<std::string> props) {
    if (props != propertiesAsUniforms) {
        propertiesAsUniforms = std::move(props);
        propertiesChanged = true;
    }
}
bool CircleLayerTweaker::hasPropertyAsUniform(const std::string_view attrName) const {
    return propertiesAsUniforms.end() !=
           std::find_if(propertiesAsUniforms.begin(), propertiesAsUniforms.end(), [&](const auto& name) {
               return name.size() + 2 == attrName.size() && 0 == std::strcmp(name.data(), attrName.data() + 2);
           });
}
#endif // MLN_RENDER_BACKEND_METAL

void CircleLayerTweaker::enableOverdrawInspector(bool value) {
    if (overdrawInspector != value) {
        overdrawInspector = value;
        propertiesChanged = true;
    }
}

void CircleLayerTweaker::execute(LayerGroupBase& layerGroup,
                                 const RenderTree& renderTree,
                                 const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& evaluated = static_cast<const CircleLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // Updated every frame, but shared across drawables
    const CirclePaintParamsUBO paintParamsUBO = {
        /* .camera_to_center_distance = */ parameters.state.getCameraToCenterDistance(),
        /* .device_pixel_ratio = */ parameters.pixelRatio,
        /* .padding = */ 0,
        0};

    if (!paintParamsUniformBuffer) {
        paintParamsUniformBuffer = context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));
    } else {
        paintParamsUniformBuffer->update(&paintParamsUBO, sizeof(CirclePaintParamsUBO));
    }

    const auto zoom = parameters.state.getZoom();

#if MLN_RENDER_BACKEND_METAL
    using ShaderClass = shaders::ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>;
    if (propertiesChanged) {
        const auto source = [this](const std::string_view& attrName) {
            return hasPropertyAsUniform(attrName) ? AttributeSource::Constant : AttributeSource::PerVertex;
        };

        const CirclePermutationUBO permutationUBO = {
            /* .color = */ {/*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{}},
            /* .radius = */ {/*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{}},
            /* .blur = */ {/*.source=*/source(ShaderClass::attributes[3].name), /*.expression=*/{}},
            /* .opacity = */ {/*.source=*/source(ShaderClass::attributes[4].name), /*.expression=*/{}},
            /* .stroke_color = */ {/*.source=*/source(ShaderClass::attributes[5].name), /*.expression=*/{}},
            /* .stroke_width = */ {/*.source=*/source(ShaderClass::attributes[6].name), /*.expression=*/{}},
            /* .stroke_opacity = */ {/*.source=*/source(ShaderClass::attributes[7].name), /*.expression=*/{}},
            /* .overdrawInspector = */ overdrawInspector,
            /* .pad = */ {0},
        };

        if (permutationUniformBuffer) {
            permutationUniformBuffer->update(&permutationUBO, sizeof(permutationUBO));
        } else {
            permutationUniformBuffer = context.createUniformBuffer(&permutationUBO, sizeof(permutationUBO));
        }
    }
    if (!expressionUniformBuffer) {
        const ExpressionInputsUBO expressionUBO = {/* .time = */ 0,
                                                   /* .frame = */ parameters.frameCount,
                                                   /* .zoom = */ static_cast<float>(zoom),
                                                   /* .pad = */ 0,
                                                   0,
                                                   0};
        expressionUniformBuffer = context.createUniformBuffer(&expressionUBO, sizeof(expressionUBO));
    }
#endif

    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;
    const bool scaleWithMap = evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map;

    // Updated only with evaluated properties
    if (!evaluatedPropsUniformBuffer) {
        const CircleEvaluatedPropsUBO evaluatedPropsUBO = {
            /* .color = */ constOrDefault<CircleColor>(evaluated),
            /* .stroke_color = */ constOrDefault<CircleStrokeColor>(evaluated),
            /* .radius = */ constOrDefault<CircleRadius>(evaluated),
            /* .blur = */ constOrDefault<CircleBlur>(evaluated),
            /* .opacity = */ constOrDefault<CircleOpacity>(evaluated),
            /* .stroke_width = */ constOrDefault<CircleStrokeWidth>(evaluated),
            /* .stroke_opacity = */ constOrDefault<CircleStrokeOpacity>(evaluated),
            /* .scale_with_map = */ scaleWithMap,
            /* .pitch_with_map = */ pitchWithMap,
            /* .padding = */ 0};
        evaluatedPropsUniformBuffer = context.createUniformBuffer(&evaluatedPropsUBO, sizeof(evaluatedPropsUBO));
    }

    layerGroup.visitDrawables([&](gfx::Drawable& drawable) {
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(MLN_STRINGIZE(CirclePaintParamsUBO), paintParamsUniformBuffer);
        uniforms.addOrReplace(MLN_STRINGIZE(CircleEvaluatedPropsUBO), evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            assert(!"Circles only render with tiles");
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(1.0f, static_cast<float>(zoom));
        const auto extrudeScale = pitchWithMap ? std::array<float, 2>{pixelsToTileUnits, pixelsToTileUnits}
                                               : parameters.pixelsToGLUnits;

        // Updated for each drawable on each frame
        const CircleDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix),
                                               /* .extrude_scale = */ extrudeScale,
                                               /* .padding = */ 0};
        uniforms.createOrUpdate(MLN_STRINGIZE(CircleDrawableUBO), &drawableUBO, context);

#if MLN_RENDER_BACKEND_METAL
        uniforms.addOrReplace(MLN_STRINGIZE(ExpressionInputsUBO), expressionUniformBuffer);
        uniforms.addOrReplace(MLN_STRINGIZE(CirclePermutationUBO), permutationUniformBuffer);
#endif // MLN_RENDER_BACKEND_METAL
    });
}

} // namespace mbgl
