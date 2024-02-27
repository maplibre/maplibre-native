#pragma once

#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/containers.hpp>

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

class TransformState;
class LayerGroupBase;
class PaintParameters;
class RenderTree;
class UnwrappedTileID;

using mat4 = std::array<double, 16>;

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

    /// Calculate matrices for this tile.
    /// @param nearClipped If true, the near plane is moved further to enhance depth buffer precision.
    /// @param inViewportPixelUnits If false, the translation is scaled based on the current zoom.
    static mat4 getTileMatrix(const UnwrappedTileID&,
                              const PaintParameters&,
                              const std::array<float, 2>& translation,
                              style::TranslateAnchorType,
                              bool nearClipped,
                              bool inViewportPixelUnits,
                              const gfx::Drawable& drawable,
                              bool aligned = false);

protected:
    /// Determine whether this tweaker should apply to the given drawable
    bool checkTweakDrawable(const gfx::Drawable&) const;

    std::string id;
    Immutable<style::LayerProperties> evaluatedProperties;

    // Indicates that the evaluated properties have changed
    bool propertiesUpdated = true;
};

} // namespace mbgl
