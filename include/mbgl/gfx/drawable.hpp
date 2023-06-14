#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/drawable_data.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace mbgl {

class Color;
class PaintParameters;
enum class RenderPass : uint8_t;

namespace gfx {

class CullFaceMode;
enum class DepthMaskType : bool;
class DrawableTweaker;
class DrawMode;
class ShaderProgramBase;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class Drawable {
public:
    /// @brief Map from sampler location to texture info
    using Textures = std::unordered_map<int32_t, gfx::Texture2DPtr>;

protected:
    Drawable(std::string name);

public:
    virtual ~Drawable();

    struct DrawSegment;
    using UniqueDrawSegment = std::unique_ptr<DrawSegment>;

    const util::SimpleIdentity& getId() const { return uniqueID; }

    /// Draw the drawable
    virtual void draw(const PaintParameters&) const = 0;

    /// Drawable name is used for debugging and troubleshooting
    const std::string& getName() const { return name; }
    void setName(std::string value) { name = std::move(value); }

    /// Which shader to use when rendering this drawable
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// The pass on which we'll be rendered
    mbgl::RenderPass getRenderPass() const { return renderPass; }
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
    void setLineWidth(int32_t value) { lineWidth = value; }

    /// @brief Remove an attached texture from this drawable at the given sampler location
    /// @param location Texture sampler location
    void removeTexture(int32_t location);

    /// @brief Return the textures attached to this drawable
    /// @return Texture and sampler location pairs
    // const Textures& getTextures() const { return textures; };

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

    using TexSourceFunc = std::function<Textures()>;

    /// @brief Provide a function to get the current textures
    void setTextureSource(TexSourceFunc value) { textureSource = std::move(value); }
    const TexSourceFunc& getTextureSource() const { return textureSource; }

    /// @brief Provide all texture sources at once
    void setTextureSources(std::vector<TexSourceFunc>);

    /// Whether the drawble should be drawn
    bool getEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    /// Whether to do stenciling (based on the Tile ID)
    bool getNeedsStencil() const { return needsStencil; }
    void setNeedsStencil(bool value) { needsStencil = value; }

    /// not used for anything yet
    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

    /// Determines depth range within the layer
    int32_t getSubLayerIndex() const { return subLayerIndex; }
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    const std::optional<OverscaledTileID>& getTileID() const { return tileID; }
    void setTileID(const OverscaledTileID& value) { tileID = value; }

    DepthMaskType getDepthType() const { return depthType; }
    void setDepthType(DepthMaskType value) { depthType = value; }

    const gfx::CullFaceMode& getCullFaceMode() const;
    void setCullFaceMode(const gfx::CullFaceMode&);

    /// Get the number of vertexes
    std::size_t getVertexCount() const { return getVertexAttributes().getMaxCount(); }

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;
    virtual void setVertexAttributes(const gfx::VertexAttributeArray&) = 0;
    virtual void setVertexAttributes(gfx::VertexAttributeArray&&) = 0;

    /// Get the tweakers attached to this drawable
    const std::vector<DrawableTweakerPtr>& getTweakers() const { return tweakers; }

    /// Get the uniform buffers attached to this drawable
    virtual const gfx::UniformBufferArray& getUniformBuffers() const = 0;
    virtual gfx::UniformBufferArray& mutableUniformBuffers() = 0;

    // Reset a single color attribute for all vertexes
    virtual void resetColor(const Color&) = 0;

    /// Convert from the odd partially-normalized color component array produced by `Color::toArray` into normalized
    /// RGBA.
    static gfx::VertexAttribute::float4 colorAttrRGBA(const Color& color) {
        const auto components = color.toArray();
        return {static_cast<float>(components[0] / 255.0),
                static_cast<float>(components[1] / 255.0),
                static_cast<float>(components[2] / 255.0),
                static_cast<float>(components[3])};
    }

    virtual const std::optional<UniqueDrawableData>& getData() const { return drawableData; }
    virtual void setData(UniqueDrawableData&& value) { drawableData = std::move(value); }

protected:
    bool enabled = true;
    bool needsStencil = false;
    std::string name;
    util::SimpleIdentity uniqueID;
    gfx::ShaderProgramBasePtr shader;
    mbgl::RenderPass renderPass;
    std::optional<OverscaledTileID> tileID;
    DrawPriority drawPriority = 0;
    int32_t lineWidth = 1;
    int32_t subLayerIndex = 0;
    DepthMaskType depthType; // = DepthMaskType::ReadOnly;
    std::optional<UniqueDrawableData> drawableData{};

    struct Impl;
    std::unique_ptr<Impl> impl;

    Textures textures;
    TexSourceFunc textureSource;
    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;
using UniqueDrawable = std::unique_ptr<Drawable>;

/// Comparator for sorting drawable pointers primarily by draw priority
struct DrawableLessByPriority {
    DrawableLessByPriority(bool descending)
        : desc(descending) {}
    bool operator()(const Drawable& left, const Drawable& right) const {
        const auto& a = desc ? right : left;
        const auto& b = desc ? left : right;
        if (a.getDrawPriority() != b.getDrawPriority()) {
            return a.getDrawPriority() < b.getDrawPriority();
        }
        return a.getId() < b.getId();
    }
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
