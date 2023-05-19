#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

#include <array>
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
    const UniqueDrawable& getCurrentDrawable(bool createIfNone);

    /// Close the current drawable, using a new one for any further work
    void flush();

    /// Get all the completed drawables
    const std::vector<UniqueDrawable>& getDrawables() const { return drawables; }

    /// Get all the completed drawables and release ownership
    std::vector<UniqueDrawable> clearDrawables() {
        std::vector<UniqueDrawable> v = std::move(drawables);
        drawables = std::vector<UniqueDrawable>{};
        return v;
    }

    /// Get the ID of the drawable we're currently working on, if any
    util::SimpleIdentity getDrawableId();

    /// The pass on which we'll be rendered
    mbgl::RenderPass getRenderPass() const { return renderPass; }
    void setRenderPass(mbgl::RenderPass value) { renderPass = value; }

    /// The draw priority assigned to generated drawables
    DrawPriority getDrawPriority() const;
    void setDrawPriority(DrawPriority);

    /// The layer index assigned to generated drawables
    int32_t getLayerIndex() const { return layerIndex; }
    void setLayerIndex(int32_t value) { layerIndex = value; }

    /// Set the draw priority on all drawables including those already generated
    void resetDrawPriority(DrawPriority);

    /// The color used for emitted vertexes
    const Color& getColor() const;
    void setColor(const Color& value);

    enum class ColorMode {
        None,
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

    /// The attribute names for automatic attributes (vertex/position and color, if color mode is not `None`)
    void setVertexAttrName(std::string value) { vertexAttrName = std::move(value); }
    void setColorAttrName(std::string value) { colorAttrName = std::move(value); }

    /// Set the matrix applied to new drawables
    void setMatrix(mat4 value) { matrix = value; }

    /// Add a triangle
    void addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    /// Add another triangle based on the previous two points
    void appendTriangle(int16_t x0, int16_t y0);

    /// Add a rectangle consisting of two triangles
    void addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

    /// A set of attribute values to be added for each vertex
    void setVertexAttributes(const VertexAttributeArray&);
    void setVertexAttributes(VertexAttributeArray&&);

    /// Add some vertex elements, returns the index of the first one added
    std::size_t addVertices(const std::vector<std::array<int16_t, 2>>& vertices,
                            std::size_t vertexOffset,
                            std::size_t vertexLength);

    /// Add lines based on existing vertices
    void addLines(const std::vector<uint16_t>& indexes,
                  std::size_t indexOffset,
                  std::size_t indexLength,
                  std::size_t baseIndex = 0);

    /// Add triangles based on existing vertices
    void addTriangles(const std::vector<uint16_t>& indexes,
                      std::size_t indexOffset,
                      std::size_t indexLength,
                      std::size_t baseIndex = 0);

    /// Add already-set-up triangles directly
    void addTriangles(const std::vector<std::array<int16_t, 2>>& vertices,
                      std::size_t vertexOffset,
                      std::size_t vertexLength,
                      const std::vector<uint16_t>& indexes,
                      std::size_t indexOffset,
                      std::size_t indexLength);

    /// Add a tweaker to be attached to each emitted drawable
    void addTweaker(DrawableTweakerPtr tweaker) { tweakers.emplace_back(std::move(tweaker)); }

protected:
    /// Create an instance of the appropriate drawable type
    virtual UniqueDrawable createDrawable() const = 0;

    /// Setup the SDK-specific aspects after all the values are present
    virtual void init() = 0;

protected:
    std::string name;
    std::string drawableName;
    std::string vertexAttrName;
    std::string colorAttrName;
    mbgl::RenderPass renderPass;
    DrawPriority drawPriority = 0;
    int32_t layerIndex = -1;
    DepthMaskType depthType = DepthMaskType::ReadOnly;
    gfx::ShaderProgramBasePtr shader;
    mat4 matrix;
    UniqueDrawable currentDrawable;
    std::vector<UniqueDrawable> drawables;
    std::vector<DrawableTweakerPtr> tweakers;
    ColorMode colorMode = ColorMode::PerVertex;
    VertexAttributeArray vertexAttrs;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gfx
} // namespace mbgl
