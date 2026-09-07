#pragma once

#include <mln/style/sky_impl.hpp>
#include <mln/util/immutable.hpp>

namespace mln {

class TransitionParameters;
class PropertyEvaluationParameters;

using TransitioningSky = style::SkyProperties::Unevaluated;
using EvaluatedSky = style::SkyProperties::PossiblyEvaluated;

class RenderSky {
public:
    explicit RenderSky(Immutable<style::Sky::Impl>);

    void transition(const TransitionParameters&);
    void evaluate(const PropertyEvaluationParameters&);
    bool hasTransition() const;

    const EvaluatedSky& getEvaluated() const;

    Immutable<style::Sky::Impl> impl;

private:
    TransitioningSky transitioning;
    EvaluatedSky evaluated;
};

} // namespace mln
