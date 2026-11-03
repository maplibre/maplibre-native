#include <mln/renderer/render_sky.hpp>

namespace mln {

RenderSky::RenderSky(Immutable<style::Sky::Impl> impl_)
    : impl(std::move(impl_)),
      transitioning(impl->properties.untransitioned()) {}

void RenderSky::transition(const TransitionParameters& parameters) {
    transitioning = impl->properties.transitioned(parameters, std::move(transitioning));
}

void RenderSky::evaluate(const PropertyEvaluationParameters& parameters) {
    evaluated = transitioning.evaluate(parameters);
}

bool RenderSky::hasTransition() const {
    return transitioning.hasTransition();
}

const EvaluatedSky& RenderSky::getEvaluated() const {
    return evaluated;
}

} // namespace mln
