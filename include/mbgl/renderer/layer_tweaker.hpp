#pragma once

#include <mbgl/util/immutable.hpp>

#include <array>
#include <memory>
#include <string>

namespace mbgl {
namespace gfx {
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx
namespace shaders {
enum class AttributeSource : int32_t;
} // namespace shaders
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

#if MLN_RENDER_BACKEND_METAL
    void setPropertiesAsUniforms(std::vector<std::string>);
    bool hasPropertyAsUniform(std::string_view) const;
    shaders::AttributeSource getAttributeSource(const std::string_view& attribName) const;
#endif // MLN_RENDER_BACKEND_METAL

    void enableOverdrawInspector(bool);

    virtual void execute(LayerGroupBase&, const RenderTree&, const PaintParameters&) = 0;

protected:
    /// Calculate matrices for this tile.
    /// @param nearClipped If true, the near plane is moved further to enhance depth buffer precision.
    /// @param inViewportPixelUnits If false, the translation is scaled based on the current zoom.
    static mat4 getTileMatrix(const UnwrappedTileID&,
                              const RenderTree&,
                              const TransformState&,
                              const std::array<float, 2>& translation,
                              style::TranslateAnchorType,
                              bool nearClipped,
                              bool inViewportPixelUnits,
                              bool aligned = false);

protected:
    std::string id;
    Immutable<style::LayerProperties> evaluatedProperties;

#if MLN_RENDER_BACKEND_METAL
    // For Metal, whether a property is provided through attribtues or uniforms is specified in
    // a uniform buffer rather than by a shader compiled with different preprocessor definitions.
    std::vector<std::string> propertiesAsUniforms;
#endif // MLN_RENDER_BACKEND_METAL

    bool propertiesChanged = true;
    bool overdrawInspector = false;
};

} // namespace mbgl
