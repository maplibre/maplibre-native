#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/style/layer_properties.hpp>

namespace mbgl {

LayerTweaker::LayerTweaker(Immutable<style::LayerProperties> properties)
    : evaluatedProperties(std::move(properties)){};

}
