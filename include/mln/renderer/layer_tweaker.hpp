#pragma once

#include <mln/map/projection_base.hpp>
#include <mln/shaders/layer_ubo.hpp>
#include <mln/shaders/shader_source.hpp>
#include <mln/util/immutable.hpp>
#include <mln/util/containers.hpp>
#include <mln/util/mat4.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace mln {

namespace gfx {
class Context;
class Drawable;
class UniformBuffer;
class UniformBufferArray;
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

    /// The projection contract for this tile: `getTileMatrix` plus the per-tile projection fields.
    static ProjectionData getProjectionData(const UnwrappedTileID&,
                                            const PaintParameters&,
                                            const std::array<float, 2>& translation,
                                            style::TranslateAnchorType,
                                            bool nearClipped,
                                            bool inViewportPixelUnits,
                                            const gfx::Drawable& drawable,
                                            bool aligned = false);

    static shaders::ProjectionUBO toProjectionUBO(const ProjectionData&);

    /// Radians per pixel on the sphere for a tile; GL JS `globeExtrudeScale`. `latitudeScale` is the cosine of the
    /// center latitude on the globe and 1 on Mercator.
    static float globeExtrudeScale(const UnwrappedTileID&, float zoom, double latitudeScale);

protected:
    /// Determine whether this tweaker should apply to the given drawable
    bool checkTweakDrawable(const gfx::Drawable&) const;

#if MLN_UBO_CONSOLIDATION
    /// Upload the per-drawable projection blocks of this layer, indexed like the drawable UBO array
    void uploadProjectionUBOs(gfx::UniformBufferArray& layerUniforms,
                              const std::vector<shaders::ProjectionUBO>&,
                              gfx::Context&);
    gfx::UniformBufferPtr projectionUniformBuffer;
#endif

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
};

} // namespace mln
