#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/identity.hpp>
#include <mbgl/util/traits.hpp>

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
class ShaderProgramBase;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using UniformBufferPtr = std::shared_ptr<UniformBuffer>;
using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

class Drawable {
protected:
    Drawable(std::string name);

public:
    virtual ~Drawable();

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
    /// If multiple render pass bits are set, all must be present.
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

    /// not used for anything yet
    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

    /// The layer index determines the drawing order
    int32_t getLayerIndex() const { return layerIndex; }
    void setLayerIndex(int32_t value) { layerIndex = value; }

    /// Determines depth range within the layer
    int32_t getSubLayerIndex() const { return subLayerIndex; }
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    std::optional<OverscaledTileID> getTileID() const { return tileID; }
    void setTileID(OverscaledTileID value) { tileID = value; }

    mat4 getMatrix() const { return matrix; }
    void setMatrix(mat4 value) { matrix = value; }

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

    virtual std::vector<std::uint16_t>& getLineIndexData() const = 0;
    virtual std::vector<std::uint16_t>& getTriangleIndexData() const = 0;

    /// Attach a tweaker to be run on this drawable for each frame
    //void addTweaker(DrawableTweakerPtr tweaker) { tweakers.emplace_back(std::move(tweaker)); }
    //template <typename TIter>
    //void addTweakers(TIter beg, TIter end) {
    //    tweakers.insert(tweakers.end(), beg, end);
    //}

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

protected:
    std::string name;
    util::SimpleIdentity uniqueID;
    gfx::ShaderProgramBasePtr shader;
    mbgl::RenderPass renderPass;
    mat4 matrix; //= matrix::identity4();
    std::optional<OverscaledTileID> tileID;
    DrawPriority drawPriority = 0;
    int32_t lineWidth = 1;
    int32_t layerIndex = -1;
    int32_t subLayerIndex = 0;
    DepthMaskType depthType;// = DepthMaskType::ReadOnly;
    
    struct Impl;
    std::unique_ptr<Impl> impl;

    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;
using UniqueDrawable = std::unique_ptr<Drawable>;

/// Comparator for sorting drawable pointers primarily by layer index
struct DrawablePtrLessByLayer {
    DrawablePtrLessByLayer(bool descending)
        : desc(descending) {}
    bool operator()(const DrawablePtr& left, const DrawablePtr& right) const {
        const auto& a = desc ? right : left;
        const auto& b = desc ? left : right;
        if (!a || !b) {
            // nulls are less than non-nulls
            return (!a && b);
        }
        if (a->getLayerIndex() != b->getLayerIndex()) {
            return a->getLayerIndex() < b->getLayerIndex();
        }
        if (a->getDrawPriority() != b->getDrawPriority()) {
            return a->getDrawPriority() < b->getDrawPriority();
        }
        return a->getId() < b->getId();
    }

private:
    bool desc;
};

struct alignas(16) DrawableUBO {
    std::array<float, 4 * 4> matrix;
    std::array<float, 2> world;
    std::array<float, 2> padding;
};
static_assert(sizeof(DrawableUBO) % 16 == 0);

} // namespace gfx
} // namespace mbgl
