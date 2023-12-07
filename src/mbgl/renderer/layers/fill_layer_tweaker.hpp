#pragma once

#include <mbgl/renderer/layer_tweaker.hpp>
#include <mbgl/util/string_indexer.hpp>

#include <string>
#include <unordered_map>

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

    static const StringIdentity idFillTilePropsUBOName;
    static const StringIdentity idFillInterpolateUBOName;
    static const StringIdentity idFillOutlineInterpolateUBOName;

    static mbgl::unordered_map<UnwrappedTileID, gfx::UniformBufferPtr> matrixCache;
#if !defined(NDEBUG)
    static int matrixCacheHits;
#endif

private:
    gfx::UniformBufferPtr fillPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePropsUniformBuffer;
    gfx::UniformBufferPtr fillPatternPropsUniformBuffer;
    gfx::UniformBufferPtr fillOutlinePatternPropsUniformBuffer;
};

} // namespace mbgl
