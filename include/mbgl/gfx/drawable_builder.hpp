#pragma once

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/gfx/polyline_generator.hpp>

#include <array>
#include <memory>
#include <vector>

namespace mbgl {

class SegmentBase;

namespace gfx {

class DrawableTweaker;
class IndexVectorBase;
class ShaderProgramBase;
class VertexAttributeArray;

using DrawableTweakerPtr = std::shared_ptr<DrawableTweaker>;
using IndexVectorBasePtr = std::shared_ptr<IndexVectorBase>;
using ShaderProgramBasePtr = std::shared_ptr<ShaderProgramBase>;
using Texture2DPtr = std::shared_ptr<Texture2D>;

/**
    Base class for drawable builders, which construct Drawables from primitives
 */
class DrawableBuilder {
protected:
    DrawableBuilder(std::string name);

public:
    /// Destructor
    virtual ~DrawableBuilder();

    /// Get the drawable we're currently working on, if any
    UniqueDrawable& getCurrentDrawable(bool createIfNone);

    /// Close the current drawable, using a new one for any further work
    void flush(gfx::Context& context);

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

    /// Set whether the drawable should be enabled
    void setEnabled(bool value) { enabled = value; }

    /// The pass on which we'll be rendered
    mbgl::RenderPass getRenderPass() const { return renderPass; }

    /// Set the pass on which the drawable will render
    void setRenderPass(mbgl::RenderPass value) { renderPass = value; }

    /// The draw priority assigned to generated drawables
    DrawPriority getDrawPriority() const;

    /// Set drawable's draw priority
    void setDrawPriority(DrawPriority);

    /// Determines depth range within the layer for 2D drawables
    int32_t getSubLayerIndex() const { return subLayerIndex; }
    void setSubLayerIndex(int32_t value) { subLayerIndex = value; }

    /// Whether to enable depth testing
    bool getEnableDepth() { return enableDepth; }
    void setEnableDepth(bool value) { enableDepth = value; }

    /// Depth writability for 2D drawables
    DepthMaskType getDepthType() const { return depthType; }

    /// Set depth type
    void setDepthType(DepthMaskType value) { depthType = value; }

    /// Uses 3D depth mode
    bool getIs3D() const { return is3D; }

    /// Set 3D mode
    void setIs3D(bool value) { is3D = value; }

    /// Set the draw priority on all drawables including those already generated
    void resetDrawPriority(DrawPriority);

    /// Width for lines
    float getLineWidth() const { return lineWidth; }

    /// Set line width
    void setLineWidth(float value) { lineWidth = value; }

    /// Whether to render to the color target
    bool getEnableColor() const { return enableColor; }

    /// Set whether to render to the color target
    void setEnableColor(bool value) { enableColor = value; }

    /// Whether to do stenciling (based on the Tile ID or 3D)
    bool getEnableStencil() const { return enableStencil; }

    /// Set stencil usage
    void setEnableStencil(bool value) { enableStencil = value; }

    /// Get color mode
    const gfx::ColorMode& getColorMode() const;

    /// Set color mode
    void setColorMode(const gfx::ColorMode& value);

    /// Get cull face mode
    const gfx::CullFaceMode& getCullFaceMode() const;

    /// Set cull face mode
    void setCullFaceMode(const gfx::CullFaceMode& value);

    /// Which shader to use when rendering emitted drawables
    const gfx::ShaderProgramBasePtr& getShader() const { return shader; }

    /// Set the shader to be used
    void setShader(gfx::ShaderProgramBasePtr value) { shader = std::move(value); }

    /// Set the name given to new drawables
    void setDrawableName(std::string value) { drawableName = std::move(value); }

    /// The attribute id for vertex/position attribute
    void setVertexAttrId(const size_t id) { vertexAttrId = id; }

    /// @brief Get the texture at the given internal ID.
    const gfx::Texture2DPtr& getTexture(size_t id) const;

    /// @brief Attach the given texture at the given internal ID.
    /// @param texture Texture2D instance
    /// @param id Internal ID of the texture.
    void setTexture(const gfx::Texture2DPtr&, size_t id);

    /// Add a tweaker to emitted drawable
    void addTweaker(DrawableTweakerPtr value) { tweakers.emplace_back(std::move(value)); }

    /// Clear the tweaker collection
    void clearTweakers() { tweakers.clear(); }

    /// Get the vertex attributes that override default values in the shader program
    VertexAttributeArrayPtr& getVertexAttributes() { return vertexAttrs; }
    const VertexAttributeArrayPtr& getVertexAttributes() const { return vertexAttrs; }

    /// A set of attribute values to be added for each vertex
    void setVertexAttributes(VertexAttributeArrayPtr attrs) { vertexAttrs = std::move(attrs); }

    /// Get the instance attributes that override default values in the shader program
    VertexAttributeArrayPtr& getInstanceAttributes() { return instanceAttrs; }
    const VertexAttributeArrayPtr& getInstanceAttributes() const { return instanceAttrs; }

    /// A set of attribute values to be added for each instance
    void setInstanceAttributes(VertexAttributeArrayPtr attrs) { instanceAttrs = std::move(attrs); }

    /// Add some vertex elements, returns the index of the first one added
    std::size_t addVertices(const std::vector<std::array<int16_t, 2>>& vertices,
                            std::size_t vertexOffset,
                            std::size_t vertexLength);

    /// Provide raw data for vertices. Incompatible with adding primitives
    void setRawVertices(std::vector<uint8_t>&&, std::size_t, AttributeDataType);

    /// Set indexes and segments
    void setSegments(gfx::DrawMode, std::vector<uint16_t> indexes, const SegmentBase*, std::size_t segmentCount);

    /// Set shared indices and segments
    void setSegments(gfx::DrawMode, gfx::IndexVectorBasePtr, const SegmentBase*, std::size_t segmentCount);

    /// Create a segment wrapper
    virtual std::unique_ptr<Drawable::DrawSegment> createSegment(gfx::DrawMode, SegmentBase&&) = 0;

    /// Add a triangle
    void addTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    /// Add another triangle based on the previous two points
    void appendTriangle(int16_t x0, int16_t y0);

    /// Add a rectangle consisting of two triangles
    void addQuad(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

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

    /// Add a polyline. If the last point equals the first it will be closed, otherwise open
    void addPolyline(const GeometryCoordinates& coordinates, const gfx::PolylineGeneratorOptions&);

    /// Add a polyline in Tile coordinates, using wide vectors.
    void addWideVectorPolylineLocal(const GeometryCoordinates& coordinates, const gfx::PolylineGeneratorOptions&);

    /// Add a polyline in geographic coordinates, using wide vectors.
    void addWideVectorPolylineGlobal(const LineString<double>& coordinates, const gfx::PolylineGeneratorOptions&);

    /// return the curent vertex count
    std::size_t curVertexCount() const;

    bool empty() const { return !(drawables.size() || curVertexCount()); }

protected:
    /// Create an instance of the appropriate drawable type
    virtual UniqueDrawable createDrawable() const = 0;

    /// Setup the SDK-specific aspects after all the values are present
    virtual void init() = 0;

protected:
    std::string name;
    std::string drawableName;
    std::size_t vertexAttrId;
    mbgl::RenderPass renderPass;
    bool enabled = true;
    bool enableColor = true;
    bool enableStencil = false;
    bool enableDepth = true;
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
    gfx::VertexAttributeArrayPtr vertexAttrs;
    gfx::VertexAttributeArrayPtr instanceAttrs;
    std::optional<mbgl::Point<double>> origin;

    class Impl;
    std::unique_ptr<Impl> impl;
};

using UniqueDrawableBuilder = std::unique_ptr<DrawableBuilder>;

} // namespace gfx
} // namespace mbgl
