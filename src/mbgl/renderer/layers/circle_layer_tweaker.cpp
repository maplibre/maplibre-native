#include <mbgl/renderer/layers/circle_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/mtl/circle.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

#include <cstring>

namespace mbgl {

using namespace style;
using namespace shaders;

void CircleLayerTweaker::setPropertiesAsUniforms(std::vector<std::string> props) {
    if (props != propertiesAsUniforms) {
        // clear buffer
        propertiesAsUniforms = std::move(props);
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
        /* .padding = */ {0}};

    if (!paintParamsUniformBuffer) {
        paintParamsUniformBuffer = context.createUniformBuffer(&paintParamsUBO, sizeof(paintParamsUBO));
    } else {
        paintParamsUniformBuffer->update(&paintParamsUBO, sizeof(CirclePaintParamsUBO));
    }

    const bool pitchWithMap = evaluated.get<CirclePitchAlignment>() == AlignmentType::Map;

    // Updated only with evaluated properties
    if (!evaluatedPropsUniformBuffer) {
        const CircleEvaluatedPropsUBO evaluatedPropsUBO = {
            /* .color = */ evaluated.get<CircleColor>().constantOr(CircleColor::defaultValue()),
            /* .stroke_color = */ evaluated.get<CircleStrokeColor>().constantOr(CircleStrokeColor::defaultValue()),
            /* .radius = */ evaluated.get<CircleRadius>().constantOr(CircleRadius::defaultValue()),
            /* .blur = */ evaluated.get<CircleBlur>().constantOr(CircleBlur::defaultValue()),
            /* .opacity = */ evaluated.get<CircleOpacity>().constantOr(CircleOpacity::defaultValue()),
            /* .stroke_width = */ evaluated.get<CircleStrokeWidth>().constantOr(CircleStrokeWidth::defaultValue()),
            /* .stroke_opacity = */
            evaluated.get<CircleStrokeOpacity>().constantOr(CircleStrokeOpacity::defaultValue()),
            /* .scale_with_map = */ evaluated.get<CirclePitchScale>() == CirclePitchScaleType::Map,
            /* .pitch_with_map = */ pitchWithMap,
            /* .padding = */ 0};
        evaluatedPropsUniformBuffer = context.createUniformBuffer(&evaluatedPropsUBO, sizeof(evaluatedPropsUBO));
    }

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.addOrReplace(MLN_STRINGIZE(CirclePaintParamsUBO), paintParamsUniformBuffer);
        uniforms.addOrReplace(MLN_STRINGIZE(CircleEvaluatedPropsUBO), evaluatedPropsUniformBuffer);

        if (!drawable.getTileID()) {
            return;
        }
        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto& translation = evaluated.get<CircleTranslate>();
        const auto anchor = evaluated.get<CircleTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = false;
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translation, anchor, nearClipped, inViewportPixelUnits);

        const auto pixelsToTileUnits = tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));

        // Updated for each drawable on each frame
        const CircleDrawableUBO drawableUBO = {
            /* .matrix = */ util::cast<float>(matrix),
            /* .extrude_scale = */
            pitchWithMap ? std::array<float, 2>{{pixelsToTileUnits}} : parameters.pixelsToGLUnits,
            /* .padding = */ {0}};

        uniforms.createOrUpdate(MLN_STRINGIZE(CircleDrawableUBO), &drawableUBO, context);

        const auto source = [&](const std::string_view& attrName) {
            const auto hit = std::find_if(propertiesAsUniforms.begin(),
                                          propertiesAsUniforms.end(), [&](const auto& name){
                return name.size() + 2 == attrName.size() &&
                        0 == std::strcmp(name.data(), attrName.data() + 2);
            });
            return (hit == propertiesAsUniforms.end()) ? AttributeSource::PerVertex : AttributeSource::Constant;
        };

        using ShaderClass = shaders::ShaderSource<BuiltIn::CircleShader, gfx::Backend::Type::Metal>;
        const CirclePermutationUBO permutationUBO = {
            /* .color = */ { /*.source=*/source(ShaderClass::attributes[1].name), /*.expression=*/{} },
            /* .radius = */ { /*.source=*/source(ShaderClass::attributes[2].name), /*.expression=*/{} },
            /* .blur = */ { /*.source=*/source(ShaderClass::attributes[3].name), /*.expression=*/{} },
            /* .opacity = */ { /*.source=*/source(ShaderClass::attributes[4].name), /*.expression=*/{} },
            /* .stroke_color = */ { /*.source=*/source(ShaderClass::attributes[5].name), /*.expression=*/{} },
            /* .stroke_width = */ { /*.source=*/source(ShaderClass::attributes[6].name), /*.expression=*/{} },
            /* .stroke_opacity = */ { /*.source=*/source(ShaderClass::attributes[7].name), /*.expression=*/{} },
            /* .overdrawInspector = */ false,
            /* .pad = */ {0},
            };
        uniforms.createOrUpdate(MLN_STRINGIZE(CirclePermutationUBO), &permutationUBO, context);

        const ExpressionInputsUBO expressionUBO = {
            /* .zoom = */ 0,
            /* .time = */ 0,
            /* .frame = */ 0,
            };
        uniforms.createOrUpdate(MLN_STRINGIZE(ExpressionInputsUBO), &expressionUBO, context);
    });
}

} // namespace mbgl
