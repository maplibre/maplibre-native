#pragma once

#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/util/immutable.hpp>

#include <array>
#include <memory>
#include <string>
#include <unordered_set>

namespace mbgl {

namespace gfx {
class Drawable;
class UniformBuffer;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
} // namespace gfx

namespace shaders {
enum class AttributeSource : int32_t;
struct ExpressionInputsUBO;
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
using StringIdentity = std::size_t;

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
    /// Build the common expression inupts UBO
    static shaders::ExpressionInputsUBO buildExpressionUBO(double zoom, uint64_t frameCount);

    /// @brief Check whether a property name exists within the previously set collection.
    bool hasPropertyAsUniform(StringIdentity) const;
    shaders::AttributeSource getAttributeSource(StringIdentity) const;

    template <shaders::BuiltIn ShaderType>
    shaders::AttributeSource getAttributeSource(size_t index) {
        using ShaderClass = shaders::ShaderSource<ShaderType, gfx::Backend::Type::Metal>;
        return getAttributeSource(ShaderClass::attributes[index].nameID);
    }
#endif // MLN_RENDER_BACKEND_METAL

    /// @brief Set the collection of attribute names which will be provided at uniform values rather than per-vertex
    /// attributes.
    /// @details These values should not have "a_" prefixes, as produced by `readDataDrivenPaintProperties`.
    void setPropertiesAsUniforms(const std::unordered_set<StringIdentity>&);
    const std::unordered_set<StringIdentity>& getPropertiesAsUniforms() const;

    void enableOverdrawInspector(bool);

    virtual void execute(LayerGroupBase&, const PaintParameters&) = 0;

    void updateProperties(Immutable<style::LayerProperties>);

    // protected:
public:
    /// Determine whether this tweaker should apply to the given drawable
    bool checkTweakDrawable(const gfx::Drawable&) const;

    /// Calculate matrices for this tile.
    /// @param nearClipped If true, the near plane is moved further to enhance depth buffer precision.
    /// @param inViewportPixelUnits If false, the translation is scaled based on the current zoom.
    static mat4 getTileMatrix(const UnwrappedTileID&,
                              const PaintParameters&,
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
    std::unordered_set<StringIdentity> propertiesAsUniforms;
#endif // MLN_RENDER_BACKEND_METAL

    // Indicates that the evaluated properties have changed
    bool propertiesUpdated = true;

    // Indicates that the properties-as-uniforms has changed
    bool permutationUpdated = true;

    bool overdrawInspector = false;
};

} // namespace mbgl
