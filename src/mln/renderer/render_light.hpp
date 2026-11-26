#pragma once

#include <mln/style/light_impl.hpp>
#include <mln/util/immutable.hpp>

namespace mln {

class TransitionParameters;
class PropertyEvaluationParameters;

using TransitioningLight = style::LightProperties::Unevaluated;
using EvaluatedLight = style::LightProperties::PossiblyEvaluated;

class RenderLight {
public:
    RenderLight(Immutable<style::Light::Impl>);

    void transition(const TransitionParameters&);
    void evaluate(const PropertyEvaluationParameters&);
    bool hasTransition() const;

    const EvaluatedLight& getEvaluated() const;

    Immutable<style::Light::Impl> impl;

private:
    TransitioningLight transitioning;
    EvaluatedLight evaluated;
};

} // namespace mln
