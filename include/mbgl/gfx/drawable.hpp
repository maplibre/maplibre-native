#pragma once

#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/identity.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace mbgl {

class Color;
class PaintParameters;

namespace gfx {

enum class DepthMaskType : bool;

class DrawableTweaker;
class ShaderProgramBase;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;

using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

class Drawable {
protected:
    Drawable();

public:
    virtual ~Drawable() = default;

    const util::SimpleIdentity& getId() const { return uniqueID; }

    /// Draw the drawable
    virtual void draw(const PaintParameters &) const = 0;

    /// Which shader to use when rendering this drawable
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// DrawPriority determines the drawing order
    DrawPriority getDrawPriority() const { return drawPriority; }
    void setDrawPriority(DrawPriority value) { drawPriority = value; }

    std::optional<OverscaledTileID> getTileID() const { return tileID; }
    void setTileID(OverscaledTileID value) { tileID = value; }

    mat4 getMatrix() const { return matrix; }
    void setMatrix(mat4 value) { matrix = value; }

    DepthMaskType getDepthType() const { return depthType; }
    void setDepthType(DepthMaskType value) { depthType = value; }
    
    /// Get the number of vertexes
    std::size_t getVertexCount() const { return getVertexAttributes().getMaxCount(); }

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;
    virtual void setVertexAttributes(const gfx::VertexAttributeArray&) = 0;
    virtual void setVertexAttributes(gfx::VertexAttributeArray&&) = 0;

    virtual std::vector<std::uint16_t>& getIndexData() const = 0;

    /// Attach a tweaker to be run on this drawable for each frame
    void addTweaker(DrawableTweakerPtr tweaker) { tweakers.emplace_back(std::move(tweaker)); }
    template <typename TIter>
    void addTweakers(TIter beg, TIter end) { tweakers.insert(tweakers.end(), beg, end); }

    /// Get the tweakers attached to this drawable
    const std::vector<DrawableTweakerPtr>& getTweakers() const { return tweakers; }

    // Reset a single color attribute for all vertexes
    virtual void resetColor(const Color&) = 0;

protected:
    util::SimpleIdentity uniqueID;
    gfx::ShaderProgramBasePtr shader;
    mat4 matrix; //= matrix::identity4();
    std::optional<OverscaledTileID> tileID;
    DrawPriority drawPriority = 0;
    DepthMaskType depthType;// = DepthMaskType::ReadOnly;

    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx
} // namespace mbgl
