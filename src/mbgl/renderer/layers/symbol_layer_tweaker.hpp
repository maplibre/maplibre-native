#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/hash.hpp>

#include <string>

namespace mbgl {

/**
    Symbol layer specific tweaker
 */
class SymbolLayerTweaker : public LayerTweaker {
public:
    SymbolLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}
    ~SymbolLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

private:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
#endif
};

} // namespace mbgl
