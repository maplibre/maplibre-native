#pragma once

#include <mbgl/util/identity.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace mbgl {

class PaintParameters;

namespace gfx {

class DrawableTweaker;
class ShaderProgramBase;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;

using DrawPriority = int64_t;
using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

class Drawable {
protected:
    Drawable() = default;

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

protected:
    util::SimpleIdentity uniqueID;
    gfx::ShaderProgramBasePtr shader;

    DrawPriority drawPriority = 0;

    std::vector<DrawableTweakerPtr> tweakers;
};

using DrawablePtr = std::shared_ptr<Drawable>;

} // namespace gfx
} // namespace mbgl
