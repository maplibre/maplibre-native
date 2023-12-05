#include <mbgl/renderer/layer_tweaker.hpp>

#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/containers.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/util/monotonic_timer.hpp>
#include <chrono>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

LayerTweaker::LayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
    : id(std::move(id_)),
      evaluatedProperties(std::move(properties)) {}

bool LayerTweaker::checkTweakDrawable(const gfx::Drawable& drawable) const {
    // Apply to a drawable if it references us, or if doesn't reference anything.
    const auto& tweaker = drawable.getLayerTweaker();
    return !tweaker || tweaker.get() == this;
}

mat4 LayerTweaker::getTileMatrix(const UnwrappedTileID& tileID,
                                 const PaintParameters& parameters,
                                 const std::array<float, 2>& translation,
                                 style::TranslateAnchorType anchor,
                                 bool nearClipped,
                                 bool inViewportPixelUnits,
                                 bool aligned) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    parameters.state.matrixFor(/*out*/ tileMatrix, tileID);

    // nearClippedMatrix has near plane moved further, to enhance depth buffer precision
    const auto& projMatrix = aligned ? parameters.transformParams.alignedProjMatrix
                                     : (nearClipped ? parameters.transformParams.nearClippedProjMatrix
                                                    : parameters.transformParams.projMatrix);
    matrix::multiply(tileMatrix, projMatrix, tileMatrix);

    return RenderTile::translateVtxMatrix(
        tileID, tileMatrix, translation, anchor, parameters.state, inViewportPixelUnits);
}

void LayerTweaker::updateProperties(Immutable<style::LayerProperties> newProps) {
    evaluatedProperties = std::move(newProps);
    propertiesUpdated = true;
}

#if MLN_RENDER_BACKEND_METAL
shaders::ExpressionInputsUBO LayerTweaker::buildExpressionUBO(double zoom, uint64_t frameCount) {
    const auto time = util::MonotonicTimer::now();
    const auto time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time).count();
    return {/* .time_lo = */ static_cast<uint32_t>(time_ns),
            /* .time_hi = */ static_cast<uint32_t>(time_ns >> 32),
            /* .frame_lo = */ static_cast<uint32_t>(frameCount),
            /* .frame_hi = */ static_cast<uint32_t>(frameCount >> 32),
            /* .zoom = */ static_cast<float>(zoom),
            /* .zoom_frac = */ static_cast<float>(zoom - static_cast<float>(zoom)),
            /* .pad = */ 0,
            0};
}
#endif // MLN_RENDER_BACKEND_METAL

void LayerTweaker::setPropertiesAsUniforms([[maybe_unused]] const mbgl::unordered_set<StringIdentity>& props) {
#if MLN_RENDER_BACKEND_METAL
    if (props != propertiesAsUniforms) {
        propertiesAsUniforms = props;
        permutationUpdated = true;
    }
#endif
}

#if !MLN_RENDER_BACKEND_METAL
namespace {
const mbgl::unordered_set<StringIdentity> emptyIDSet;
}
#endif

const mbgl::unordered_set<StringIdentity>& LayerTweaker::getPropertiesAsUniforms() const {
#if MLN_RENDER_BACKEND_METAL
    return propertiesAsUniforms;
#else
    return emptyIDSet;
#endif
}

void LayerTweaker::enableOverdrawInspector(bool value) {
    if (overdrawInspector != value) {
        overdrawInspector = value;
        permutationUpdated = true;
    }
}

} // namespace mbgl
