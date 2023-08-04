#include <mbgl/renderer/layers/render_debug_layer.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/pattern_atlas.hpp>
#include <mbgl/renderer/render_pass.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/layers/Debug_layer_impl.hpp>
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/util/tile_cover.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/logging.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/renderer/change_request.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif

#include <unordered_set>

namespace mbgl {

using namespace style;

namespace {

inline const DebugLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == DebugLayer::Impl::staticTypeInfo());
    return static_cast<const style::DebugLayer::Impl&>(*impl);
}

} // namespace

RenderDebugLayer::RenderDebugLayer(Immutable<style::DebugLayer::Impl> _impl)
    : RenderLayer(makeMutable<DebugLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderDebugLayer::~RenderDebugLayer() = default;

void RenderDebugLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderDebugLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto evaluated = unevaluated.evaluate(parameters);
    auto properties = makeMutable<DebugLayerProperties>(staticImmutableCast<DebugLayer::Impl>(baseImpl));

    passes = RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);

    evaluatedProperties = std::move(properties);
}

bool RenderDebugLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderDebugLayer::hasCrossfade() const {
    return false;
}

#if MLN_DRAWABLE_RENDERER
static constexpr std::string_view DebugShaderName = "DebugShader";

void RenderDebugLayer::update(gfx::ShaderRegistry& shaders,
                                   gfx::Context& context,
                                   const TransformState& state,
                                   [[maybe_unused]] const RenderTree& renderTree,
                                   [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);
}
#endif

} // namespace mbgl
