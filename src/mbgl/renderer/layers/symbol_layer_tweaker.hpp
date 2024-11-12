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

    // Interpolation UBOs are shared by drawables of the same type (text/icon) in each tile
    struct InterpUBOKey {
        UnwrappedTileID tileID;
        bool isText;

        bool operator==(const InterpUBOKey& other) const { return isText == other.isText && tileID == other.tileID; }
    };
    struct InterpUBOValue {
        gfx::UniformBufferPtr ubo;
        uint64_t updatedFrame = 0;
    };
    struct InterpUBOHash {
        size_t operator()(const InterpUBOKey& k) const { return util::hash(k.tileID, k.isText); }
    };
    mbgl::unordered_map<InterpUBOKey, InterpUBOValue, InterpUBOHash> interpUBOs;
};

} // namespace mbgl
