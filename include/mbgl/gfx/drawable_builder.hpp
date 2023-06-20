#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>

#include <array>
#include <memory>
#include <vector>

namespace mbgl {

class SegmentBase;

namespace gfx {

class DrawableTweaker;
class ShaderProgramBase;

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
    int32_t getSubLayerIndex() const { return subLayerIndex; }
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    /// Set the draw priority on all drawables including those already generated
    void resetDrawPriority(DrawPriority);

    /// Width for lines
    float getLineWidth() const { return lineWidth; }
    void setLineWidth(float value) { lineWidth = value; }

    /// Whether to do stenciling (based on the Tile ID)
    bool getNeedsStencil() const { return needsStencil; }
    void setNeedsStencil(bool value) { needsStencil = value; }

    DepthMaskType getDepthType() const { return depthType; }
    void setDepthType(DepthMaskType value) { depthType = value; }

    const gfx::CullFaceMode& getCullFaceMode() const;
    void setCullFaceMode(const gfx::CullFaceMode& value);

    /// Which shader to use when rendering emitted drawables
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// Get the vertex attributes that override default values in the shader program
    virtual const gfx::VertexAttributeArray& getVertexAttributes() const = 0;

    /// Set the name given to new drawables
    void setDrawableName(std::string value) { drawableName = std::move(value); }

    /// The attribute names for vertex/position attributes
    void setVertexAttrName(std::string value) { vertexAttrName = std::move(value); }

    /// @brief Attach the given texture at the given sampler location.
    /// @param texture Texture2D instance
    /// @param location A sampler location in the shader being used.
    void setTexture(const gfx::Texture2DPtr&, int32_t location);

    /// @brief Provide a function to get the current texture
    void setTextureSource(Drawable::TexSourceFunc value) { textureSource = std::move(value); }

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

protected:
    std::size_t curVertexCount() const;

    /// Create an instance of the appropriate drawable type
    virtual UniqueDrawable createDrawable() const = 0;

    /// Create a segment wrapper
    virtual std::unique_ptr<Drawable::DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) = 0;

    /// Setup the SDK-specific aspects after all the values are present
    virtual void init() = 0;

protected:
    std::string name;
    std::string drawableName;
    std::string vertexAttrName;
    mbgl::RenderPass renderPass;
    bool needsStencil = false;
    float lineWidth = 1.0f;
    DrawPriority drawPriority = 0;
    int32_t subLayerIndex = 0;
    DepthMaskType depthType = DepthMaskType::ReadOnly;
    gfx::ShaderProgramBasePtr shader;
    UniqueDrawable currentDrawable;
    std::vector<UniqueDrawable> drawables;
    VertexAttributeArray vertexAttrs;
    gfx::Drawable::Textures textures;
    Drawable::TexSourceFunc textureSource;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gfx
} // namespace mbgl
