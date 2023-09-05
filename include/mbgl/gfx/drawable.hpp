#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/gfx/index_vector.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace mbgl {

class Color;
class PaintParameters;
enum class RenderPass : uint8_t;

namespace gfx {

class ColorMode;
class CullFaceMode;
enum class DepthMaskType : bool;
class DrawableTweaker;
class DrawMode;
class IndexVectorBase;
class ShaderProgramBase;
class VertexAttributeArray;

using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class Drawable {
public:
    /// @brief Map from sampler location to texture info
    using Textures = std::unordered_map<int32_t, gfx::Texture2DPtr>;

protected:
    Drawable(std::string name);

public:
    /// Drawable constructor
    virtual ~Drawable();

    struct DrawSegment;
    using UniqueDrawSegment = std::unique_ptr<DrawSegment>;

    /// Get drawable's ID
    const util::SimpleIdentity& getID() const { return uniqueID; }

    /// Draw the drawable
    virtual void draw(PaintParameters&) const = 0;

    /// Drawable name is used for debugging and troubleshooting
    const std::string& getName() const { return name; }

    /// Set drawable name
    void setName(std::string value) { name = std::move(value); }

    /// Which shader to use when rendering this drawable
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }

    /// Set the shader to be used
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// The pass on which we'll be rendered
    mbgl::RenderPass getRenderPass() const { return renderPass; }

    /// Sets the render passes
    void setRenderPass(mbgl::RenderPass value) { renderPass = value; }

    /// Test whether to draw this drawable in a given render pass.
    bool hasRenderPass(const mbgl::RenderPass value) const {
        return (mbgl::underlying_type(renderPass) & mbgl::underlying_type(value)) != 0;
    }

    /// Test whether to draw this drawable in a given render pass.
    /// If multiple render pass bits are set, all must be present.
    bool hasAllRenderPasses(const mbgl::RenderPass value) const {
        const auto underlying_value = mbgl::underlying_type(value);
        return (mbgl::underlying_type(renderPass) & underlying_value) == underlying_value;
    }

    /// Width for lines
    int32_t getLineWidth() const { return lineWidth; }

    /// Set line width
    void setLineWidth(int32_t value) { lineWidth = value; }

    /// @brief Remove an attached texture from this drawable at the given sampler location
    /// @param location Texture sampler location
    void removeTexture(int32_t location);

    /// @brief Return the textures attached to this drawable
    /// @return Texture and sampler location pairs
    const Textures& getTextures() const { return textures; };

    /// @brief Get the texture at the given sampler location.
    const gfx::Texture2DPtr& getTexture(int32_t location) const;

    /// @brief Set the collection of textures bound to this drawable
    /// @param textures_ A Textures collection to set
    void setTextures(const Textures& textures_) noexcept { textures = textures_; }
    void setTextures(Textures&& textures_) noexcept { textures = std::move(textures_); }

    /// @brief Attach the given texture to this drawable at the given sampler location.
    /// @param texture Texture2D instance
    /// @param location A sampler location in the shader being used with this drawable.
    void setTexture(gfx::Texture2DPtr texture, int32_t location);

    /// Whether the drawble should be drawn
    bool getEnabled() const { return enabled; }

    /// Sets whether the drawble should be drawn
    void setEnabled(bool value) { enabled = value; }

    /// Whether to render to the color target
    bool getEnableColor() const { return enableColor; }

    /// Set whether to render to the color target
    void setEnableColor(bool value) { enableColor = value; }

    /// Whether to do stenciling (based on the Tile ID or 3D)
    bool getEnableStencil() const { return enableStencil; }

    /// Set stencil usage
    void setEnableStencil(bool value) { enableStencil = value; }

    /// not used for anything yet
    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

    /// Get sub-layer index. Determines depth range within the layer for 2D drawables
    int32_t getSubLayerIndex() const { return subLayerIndex; }

    /// Set sub-layer index
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    /// Depth writability for 2D drawables
    DepthMaskType getDepthType() const { return depthType; }

    /// Set depth type
    void setDepthType(DepthMaskType value) { depthType = value; }

    /// Uses 3D depth mode
    bool getIs3D() const { return is3D; }

    /// Set 3D mode
    void setIs3D(bool value) { is3D = value; }

    /// True if this is a custom drawable
    bool getIsCustom() const { return isCustom; }

    /// Sets custom status for this drawable
    void setIsCustom(bool value) { isCustom = value; }

    /// Get the ID of the tile that this drawable represents, if any
    const std::optional<OverscaledTileID>& getTileID() const { return tileID; }

    /// Set the ID of the tile that this drawable represents
    void setTileID(const OverscaledTileID& value) { tileID = value; }

    /// Get cull face mode
    const gfx::CullFaceMode& getCullFaceMode() const;

    /// Set cull face mode
    void setCullFaceMode(const gfx::CullFaceMode&);

    /// Get color mode
    const gfx::ColorMode& getColorMode() const;

    /// Set color mode
    void setColorMode(const gfx::ColorMode&);

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    /// Get the mutable vertex attribute array
    virtual gfx::VertexAttributeArray& mutableVertexAttributes() = 0;

    /// Set vertex attribute array
    virtual void setVertexAttributes(const gfx::VertexAttributeArray&) = 0;

    /// Set vertex attribute array
    virtual void setVertexAttributes(gfx::VertexAttributeArray&&) = 0;

    /// Provide raw data for vertices. Incompatible with adding primitives
    virtual void setVertices(std::vector<uint8_t>&&, std::size_t, AttributeDataType) = 0;

    /// Provide raw indexes and segments
    void setIndexData(std::vector<std::uint16_t> indexes, std::vector<UniqueDrawSegment>);

    /// Set shared indexes and segments
    virtual void setIndexData(gfx::IndexVectorBasePtr, std::vector<UniqueDrawSegment>) = 0;

    /// Get the tweakers attached to this drawable
    const std::vector<DrawableTweakerPtr>& getTweakers() const { return tweakers; }

    /// Add a tweaker to this drawable
    void addTweaker(DrawableTweakerPtr value) { tweakers.emplace_back(std::move(value)); }

    /// Set the entire tweaker collection
    void setTweakers(std::vector<DrawableTweakerPtr> value) { tweakers = std::move(value); }

    /// Clear the tweaker collection
    void clearTweakers() { tweakers.clear(); }

    /// Get the uniform buffers attached to this drawable
    virtual const gfx::UniformBufferArray& getUniformBuffers() const = 0;

    /// Get the mutable uniform buffer array
    virtual gfx::UniformBufferArray& mutableUniformBuffers() = 0;

    /// Get drawable data
    const UniqueDrawableData& getData() const { return drawableData; }

    /// Set drawable data
    void setData(UniqueDrawableData&& value) { drawableData = std::move(value); }

protected:
    bool enabled = true;
    bool enableColor = true;
    bool enableStencil = false;
    bool is3D = false;
    bool isCustom = false;
    std::string name;
    const util::SimpleIdentity uniqueID;
    gfx::ShaderProgramBasePtr shader;
    mbgl::RenderPass renderPass;
    std::optional<OverscaledTileID> tileID;
    DrawPriority drawPriority = 0;
    int32_t lineWidth = 1;
    int32_t subLayerIndex = 0;
    DepthMaskType depthType; // = DepthMaskType::ReadOnly;
    UniqueDrawableData drawableData{};

    struct Impl;
    std::unique_ptr<Impl> impl;

    Textures textures;
    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;
using UniqueDrawable = std::unique_ptr<Drawable>;

/// Comparator for sorting drawable pointers primarily by draw priority
struct DrawableLessByPriority {
    DrawableLessByPriority(bool descending = false)
        : desc(descending) {}
    bool operator()(const Drawable& left, const Drawable& right) const {
        const auto& a = desc ? right : left;
        const auto& b = desc ? left : right;
        if (a.getDrawPriority() != b.getDrawPriority()) {
            return a.getDrawPriority() < b.getDrawPriority();
        }
        return a.getID() < b.getID();
    }
    bool operator()(const Drawable* left, const Drawable* right) const { return operator()(*left, *right); }
    bool operator()(const UniqueDrawable& left, const UniqueDrawable& right) const { return operator()(*left, *right); }
    bool operator()(const DrawablePtr& left, const DrawablePtr& right) const {
        const auto& a = desc ? right : left;
        const auto& b = desc ? left : right;
        // nulls are less than non-nulls
        return (a && b) ? operator()(*a, *b) : (!a && b);
    }

private:
    bool desc;
};

} // namespace gfx
} // namespace mbgl
