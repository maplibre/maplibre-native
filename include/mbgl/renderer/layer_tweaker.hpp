#pragma once

#include <mbgl/util/immutable.hpp>

#include <memory>

namespace mbgl {

namespace style {
class LayerProperties;
} // namespace style

class PaintParameters;
class LayerGroup;

/**
    Base class for layer tweakers, which manipulate layer group per frame
 */
class LayerTweaker {
protected:
    LayerTweaker(Immutable<style::LayerProperties> properties);

public:
    LayerTweaker() = delete;
    virtual ~LayerTweaker() = default;

    virtual void execute(LayerGroup&, const PaintParameters&) = 0;

protected:
    Immutable<style::LayerProperties> evaluatedProperties;
};

} // namespace mbgl
