#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/shaders/color_relief_layer_ubo.hpp>

namespace mbgl {

class ColorReliefLayerTweaker : public LayerTweaker {
public:
    ColorReliefLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~ColorReliefLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    shaders::ColorReliefDrawableUBO drawableUBO;
    shaders::ColorReliefTilePropsUBO tilePropsUBO;
    shaders::ColorReliefEvaluatedPropsUBO evaluatedPropsUBO;
};

} // namespace mbgl
