#pragma once

#include <mln/renderer/layer_tweaker.hpp>

#include <string>

#if MLN_UBO_CONSOLIDATION
#include <mln/util/identity.hpp>
#include <vector>
#endif

namespace mln {

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
    gfx::UniformBufferPtr evaluatedPropsUniformBuffer;

#if MLN_UBO_CONSOLIDATION
    gfx::UniformBufferPtr drawableUniformBuffer;
    gfx::UniformBufferPtr tilePropsUniformBuffer;
    gfx::UniformBufferPtr featureUniformBuffer;
    // Ordered bucket ids from the previous frame. `feature_base` is an offset into that concatenation.
    std::vector<util::SimpleIdentity> featureBinderOrder;
#endif
};

} // namespace mln
