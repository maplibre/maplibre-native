#include <mbgl/renderer/layers/collision_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/collision_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

const StringIdentity CollisionLayerTweaker::idCollisionCircleUBOName = stringIndexer().get(CollisionCircleUBOName);
const StringIdentity CollisionLayerTweaker::idCollisionBoxUBOName = stringIndexer().get(CollisionBoxUBOName);

void CollisionLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData() || !checkTweakDrawable(drawable)) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& data = static_cast<gfx::CollisionDrawableData&>(*drawable.getData());

        // matrix
        const auto translate = data.translate;
        const auto anchor = data.translateAnchor;
        constexpr bool nearClipped = false;
        constexpr bool inViewportPixelUnits = false;
        const auto matrix = getTileMatrix(
            tileID, parameters, translate, anchor, nearClipped, inViewportPixelUnits, drawable);

        // extrude scale
        const auto pixelRatio = tileID.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));
        const auto scale = static_cast<float>(
            std::pow(2, parameters.state.getZoom() - drawable.getTileID()->overscaledZ));
        const std::array<float, 2> extrudeScale = {{parameters.pixelsToGLUnits[0] / (pixelRatio * scale),
                                                    parameters.pixelsToGLUnits[1] / (pixelRatio * scale)}};

        const CollisionUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
            /*.extrude_scale*/ extrudeScale,
            /*.camera_to_center_distance*/ parameters.state.getCameraToCenterDistance(),
            /*.overscale_factor*/ static_cast<float>(drawable.getTileID()->overscaleFactor())};

        const auto shader = drawable.getShader();
        const auto& shaderUniforms = shader->getUniformBlocks();
        auto& uniforms = drawable.mutableUniformBuffers();

        if (shaderUniforms.get(idCollisionBoxUBOName)) {
            // collision box
            uniforms.createOrUpdate(idCollisionBoxUBOName, &drawableUBO, context);
        } else if (shaderUniforms.get(idCollisionCircleUBOName)) {
            // collision circle
            uniforms.createOrUpdate(idCollisionCircleUBOName, &drawableUBO, context);
        } else {
            Log::Error(Event::General, "Collision shader uniform name unknown.");
        }
    });
}

} // namespace mbgl
