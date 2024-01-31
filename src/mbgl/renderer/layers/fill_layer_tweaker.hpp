#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <string>

namespace mbgl {

/**
    Fill layer specific tweaker
 */
class FillLayerTweaker : public LayerTweaker {
public:
    FillLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~FillLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    gfx::UniformBufferPtr fillPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePropsUniformBuffer;
    gfx::UniformBufferPtr fillPatternPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPropsUniformBuffer;
};

} // namespace mbgl
