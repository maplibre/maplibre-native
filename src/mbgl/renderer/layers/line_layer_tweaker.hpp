#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>

#include <string_view>

namespace mbgl {

namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

/**
    Line layer specific tweaker
 */
class LineLayerTweaker : public LayerTweaker {
public:
    enum class LineType {
        Simple,
        Gradient,
        Pattern,
        SDF
    };

    LineLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

    ~LineLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;
};

} // namespace mbgl
