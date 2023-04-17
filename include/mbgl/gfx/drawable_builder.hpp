#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class Color;

namespace gfx {

class DrawableTweaker;

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;

/**
    Base class for drawable builders, which construct Drawables from primitives
 */
class DrawableBuilder {
protected:
    DrawableBuilder();

public:
    virtual ~DrawableBuilder();

    /// Get the drawable we're currently working on, if any
    DrawablePtr getCurrentDrawable(bool createIfNone);

    /// Close the current drawable, using a new one for any further work
    void flush();

    /// Get all the completed drawables
    const std::vector<DrawablePtr>& getDrawables() const { return drawables; }

    /// Get all the completed drawables and release ownership
    std::vector<DrawablePtr> clearDrawables() const { return std::move(drawables); }

    /// Get the ID of the drawable we're currently working on, if any
    util::SimpleIdentity getDrawableId();

    /// Get the draw priority assigned to generated drawables
    DrawPriority getDrawPriority() const;
    /// Set the draw priority assigned to generated drawables
    void setDrawPriority(DrawPriority);

    /// Set the draw priority on all drawables including those already generated
    void resetDrawPriority(DrawPriority);

    /// The color used for emitted vertexes
    const Color& getColor() const;
    void setColor(const Color& value);

    /// Which shader to use when rendering emitted drawables
    //util::SimpleIdentity getShaderID() const { return shaderID; }
    //void setShaderID(util::SimpleIdentity value) { shaderID = value; }
    const std::string& getShaderID() const { return shaderID; }
    void setShaderID(std::string value) { shaderID = std::move(value); }

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    /// Add a triangle
    void addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    /// Add another triangle based on the previous two points
    void appendTriangle(int16_t x0, int16_t y0);

    /// Add a rectangle consisting of two triangles
    void addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

    /// Add a tweaker to be attached to each emitted drawable
    void addTweaker(DrawableTweakerPtr tweaker) { tweakers.emplace_back(std::move(tweaker)); }

protected:
    /// Create an instance of the appropriate drawable type
    virtual DrawablePtr createDrawable() const = 0;

    /// Setup the SDK-specific aspects after all the values are present
    virtual void init() = 0;

protected:
    DrawPriority drawPriority = 0;

    //util::SimpleIdentity shaderID = util::SimpleIdentity::Empty;
    std::string shaderID;

    DrawablePtr currentDrawable;
    std::vector<DrawablePtr> drawables;

    std::vector<DrawableTweakerPtr> tweakers;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gfx
} // namespace mbgl
