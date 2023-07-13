#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>

#include <array>
#include <memory>
#include <vector>

namespace mbgl {

class SegmentBase;

namespace gfx {

class DrawableTweaker;
class ShaderProgramBase;
class VertexAttributeArray;

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using Texture2DPtr = std::shared_ptr<Texture2D>;

/**
    Base class for drawable builders, which construct Drawables from primitives
 */
class DrawableBuilder {
protected:
    DrawableBuilder(std::string name);

public:
    virtual ~DrawableBuilder();

    /// Get the drawable we're currently working on, if any
    UniqueDrawable& getCurrentDrawable(bool createIfNone);

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

    /// Whether the drawble should be drawn
    bool getEnabled() const { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    /// The pass on which we'll be rendered
    mbgl::RenderPass getRenderPass() const { return renderPass; }
    void setRenderPass(mbgl::RenderPass value) { renderPass = value; }

    /// The draw priority assigned to generated drawables
    DrawPriority getDrawPriority() const;
    void setDrawPriority(DrawPriority);

    /// Determines depth range within the layer for 2D drawables
    int32_t getSubLayerIndex() const { return subLayerIndex; }
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    /// Depth writability for 2D drawables
    DepthMaskType getDepthType() const { return depthType; }
    void setDepthType(DepthMaskType value) { depthType = value; }

    /// Uses 3D depth mode
    bool getIs3D() const { return is3D; }
    void setIs3D(bool value) { is3D = value; }

    /// Set the draw priority on all drawables including those already generated
    void resetDrawPriority(DrawPriority);

    /// Width for lines
    float getLineWidth() const { return lineWidth; }
    void setLineWidth(float value) { lineWidth = value; }

    /// Whether to render to the color target
    bool getEnableColor() const { return enableColor; }
    void setEnableColor(bool value) { enableColor = value; }

    /// Whether to do stenciling (based on the Tile ID or 3D)
    bool getEnableStencil() const { return enableStencil; }
    void setEnableStencil(bool value) { enableStencil = value; }

    const gfx::ColorMode& getColorMode() const;
    void setColorMode(const gfx::ColorMode& value);

    const gfx::CullFaceMode& getCullFaceMode() const;
    void setCullFaceMode(const gfx::CullFaceMode& value);

    /// Which shader to use when rendering emitted drawables
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// Get the vertex attributes that override default values in the shader program
    const gfx::VertexAttributeArray& getVertexAttributes() const;

    /// Set the name given to new drawables
    void setDrawableName(std::string value) { drawableName = std::move(value); }

    /// The attribute names for vertex/position attributes
    void setVertexAttrName(std::string value) { vertexAttrName = std::move(value); }

    /// @brief Attach the given texture at the given sampler location.
    /// @param texture Texture2D instance
    /// @param location A sampler location in the shader being used.
    void setTexture(const gfx::Texture2DPtr&, int32_t location);

    void addTweaker(DrawableTweakerPtr value) { tweakers.emplace_back(std::move(value)); }
    void clearTweakers() { tweakers.clear(); }

    /// Add a triangle
    void addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    /// Add another triangle based on the previous two points
    void appendTriangle(int16_t x0, int16_t y0);

    /// Add a rectangle consisting of two triangles
    void addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

    /// A set of attribute values to be added for each vertex
    void setVertexAttributes(const VertexAttributeArray& value);
    void setVertexAttributes(VertexAttributeArray&&);

    /// Add some vertex elements, returns the index of the first one added
    std::size_t addVertices(const std::vector<std::array<int16_t, 2>>& vertices,
                            std::size_t vertexOffset,
                            std::size_t vertexLength);

    /// Provide raw data for vertices
    /// Incompatible with adding primitives
    void setRawVertices(std::vector<uint8_t>&&, std::size_t, AttributeDataType);

    void setSegments(gfx::DrawMode, std::vector<uint16_t> indexes, const std::vector<SegmentBase>&);
    void setSegments(gfx::DrawMode, std::vector<uint16_t> indexes, const SegmentBase*, std::size_t segmentCount);

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

    /// Create a segment wrapper
    virtual std::unique_ptr<Drawable::DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) = 0;

protected:
    std::size_t curVertexCount() const;

    /// Create an instance of the appropriate drawable type
    virtual UniqueDrawable createDrawable() const = 0;

    /// Setup the SDK-specific aspects after all the values are present
    virtual void init() = 0;

protected:
    std::string name;
    std::string drawableName;
    std::string vertexAttrName;
    mbgl::RenderPass renderPass;
    bool enabled = true;
    bool enableColor = true;
    bool enableStencil = false;
    bool is3D = false;
    float lineWidth = 1.0f;
    DrawPriority drawPriority = 0;
    int32_t subLayerIndex = 0;
    DepthMaskType depthType = DepthMaskType::ReadOnly;
    gfx::ShaderProgramBasePtr shader;
    UniqueDrawable currentDrawable;
    std::vector<UniqueDrawable> drawables;
    gfx::Drawable::Textures textures;
    std::vector<DrawableTweakerPtr> tweakers;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gfx
} // namespace mbgl
