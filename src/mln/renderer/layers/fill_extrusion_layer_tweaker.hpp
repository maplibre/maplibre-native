#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <string>

#if MLN_UBO_CONSOLIDATION
#include <mln/util/identity.hpp>
#include <vector>
#endif

namespace mln {

/**
    Fill extrusion layer specific tweaker
 */
class FillExtrusionLayerTweaker : public LayerTweaker {
public:
    FillExtrusionLayerTweaker(std::string id_, Immutable<style::LayerProperties> properties)
        : LayerTweaker(std::move(id_), properties) {}

public:
    ~FillExtrusionLayerTweaker() override = default;

    void execute(LayerGroupBase&, const PaintParameters&) override;

protected:
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
    gfx::UniformBufferPtr featureUniformBuffer;
    std::vector<util::SimpleIdentity> featureBinderOrder;
#endif
};

} // namespace mln
