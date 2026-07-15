#pragma once

#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/mat4.hpp>

#include <array>
#include <memory>
#include <string>

namespace mbgl {

namespace gfx {
class Drawable;
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

namespace style {
class LayerProperties;
enum class TranslateAnchorType : bool;
} // namespace style

class LayerGroupBase;
class PaintParameters;
class RenderTree;
class TransformState;
class UnwrappedTileID;

/**
    Base class for layer tweakers, which manipulate layer group per frame
 */
class LayerTweaker {
protected:
    LayerTweaker(std::string id, Immutable<style::LayerProperties> properties);

public:
    LayerTweaker() = delete;
    virtual ~LayerTweaker() = default;

    const std::string& getID() const { return id; }

    virtual void execute(LayerGroupBase&, const PaintParameters&) = 0;

    void updateProperties(Immutable<style::LayerProperties>);

    /// Bumped whenever any layer's evaluated properties change. RenderLayer::evaluate
    /// (the only caller of updateProperties) runs solely on a style edit, a zoom change
    /// the layer depends on, a crossfade, or a running transition - never during a plain
    /// pan - which makes this a precise invalidation signal for content cached from
    /// evaluated properties, as maplibre-gl-js does with invalidateRenderCache.
    /// See RenderTarget::render, which caches each terrain drape target's texture.
    static uint64_t getPropertiesEpoch() { return propertiesEpoch; }

    /// Calculate matrices for this tile.
    /// @param nearClipped If true, the near plane is moved further to enhance depth buffer precision.
    /// @param inViewportPixelUnits If false, the translation is scaled based on the current zoom.
    /// @param renderingToTerrain If provided, set to whether the returned matrix targets a
    /// terrain render-to-texture tile instead of the map projection.
    static mat4 getTileMatrix(const UnwrappedTileID&,
                              const PaintParameters&,
                              const std::array<float, 2>& translation,
                              style::TranslateAnchorType,
                              bool nearClipped,
                              bool inViewportPixelUnits,
                              const gfx::Drawable& drawable,
                              bool aligned = false,
                              bool renderToTerrain = true,
                              bool* renderingToTerrain = nullptr);

protected:
    /// Determine whether this tweaker should apply to the given drawable
    bool checkTweakDrawable(const gfx::Drawable&) const;

    /// Multiplies with the projection matrix (either default, near clipped or aligned) for the given drawable
    static void multiplyWithProjectionMatrix(/*in-out*/ mat4& matrix,
                                             const PaintParameters& parameters,
                                             const gfx::Drawable& drawable,
                                             bool nearClipped,
                                             bool aligned);

    std::string id;
    Immutable<style::LayerProperties> evaluatedProperties;

    // Indicates that the evaluated properties have changed
    bool propertiesUpdated = true;

    // See getPropertiesEpoch. Rendering is single-threaded per frame; multiple maps
    // in one process share the counter, which only over-invalidates (never stales).
    static uint64_t propertiesEpoch;
};

} // namespace mbgl
