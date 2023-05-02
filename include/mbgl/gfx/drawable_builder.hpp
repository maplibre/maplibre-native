#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class Color;

namespace gfx {

class DrawableTweaker;
class ShaderProgramBase;

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;

/**
    Base class for drawable builders, which construct Drawables from primitives
 */
class DrawableBuilder {
protected:
    DrawableBuilder(std::string name);

public:
    virtual ~DrawableBuilder();

    /// Get the drawable we're currently working on, if any
    DrawablePtr getCurrentDrawable(bool createIfNone);

    /// Close the current drawable, using a new one for any further work
    void flush();

    /// Get all the completed drawables
    const std::vector<DrawablePtr>& getDrawables() const { return drawables; }

    /// Get all the completed drawables and release ownership
    std::vector<DrawablePtr> clearDrawables() {
        std::vector<DrawablePtr> v = std::move(drawables);
        drawables = {};
        return v;
    }

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

    enum class ColorMode {
        PerDrawable,
        PerVertex
    };
    /// Set how the color value is used
    /// This should not be changed while a build is in progress
    void setColorMode(ColorMode mode) { colorMode = mode; }

    DepthMaskType getDepthType() const { return depthType; }
    void setDepthType(DepthMaskType value) { depthType = value; }

    /// Which shader to use when rendering emitted drawables
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    /// Set the name given to new drawables
    void setDrawableName(std::string value) { drawableName = std::move(value); }

    /// Set the matrix applied to new drawables
    void setMatrix(mat4 value) { matrix = value; }

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
    std::string name;
    std::string drawableName;
    DrawPriority drawPriority = 0;
    DepthMaskType depthType = DepthMaskType::ReadOnly;
    gfx::ShaderProgramBasePtr shader;
    mat4 matrix;
    DrawablePtr currentDrawable;
    std::vector<DrawablePtr> drawables;
    std::vector<DrawableTweakerPtr> tweakers;
    ColorMode colorMode = ColorMode::PerVertex;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gfx
} // namespace mbgl
