#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/polyline_generator.hpp>
#include <mbgl/shaders/widevector_ubo.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>

namespace mbgl {
namespace gfx {

class DrawableBuilder::Impl {
public:
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t, 2>>;
    enum class Mode {
        Primitives,       ///< building primitive drawables. Not implemented
        Polylines,        ///< building drawables for thick polylines
        WideVectorLocal,  ///< building drawables for thick polylines using wide vectors in local coordinates
        WideVectorGlobal, ///< building drawables for thick polylines using wide vectors in global coordinates
        Custom            ///< building custom drawables.
    };
    struct LineLayoutVertex {
        std::array<int16_t, 2> a1;
        std::array<uint8_t, 4> a2;
    };

public:
    gfx::VertexVector<VT> vertices;

    std::vector<uint8_t> rawVertices;
    std::size_t rawVerticesCount = 0;

    gfx::VertexVector<LineLayoutVertex> polylineVertices;
    gfx::IndexVector<gfx::Triangles> polylineIndexes;

    std::shared_ptr<gfx::VertexVector<shaders::VertexTriWideVecB>> wideVectorVertices;
    gfx::VertexVector<shaders::VertexTriWideVecInstance> wideVectorInstanceData;
    std::shared_ptr<gfx::IndexVector<gfx::Triangles>> wideVectorIndexes;

    std::vector<uint16_t> buildIndexes;
    std::shared_ptr<gfx::IndexVectorBase> sharedIndexes;
    std::vector<std::unique_ptr<Drawable::DrawSegment>> segments;

    AttributeDataType rawVerticesType = static_cast<AttributeDataType>(-1);
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();

    // methods
    void addPolyline(gfx::DrawableBuilder& builder,
                     const GeometryCoordinates& coordinates,
                     const gfx::PolylineGeneratorOptions& options);

    void setupForPolylines(gfx::Context&, gfx::DrawableBuilder&);

    void addWideVectorPolylineLocal(gfx::DrawableBuilder& builder,
                                    const GeometryCoordinates& coordinates,
                                    const gfx::PolylineGeneratorOptions& options);

    mbgl::Point<double> addWideVectorPolylineGlobal(gfx::DrawableBuilder& builder,
                                                    const LineString<double>& coordinates,
                                                    const gfx::PolylineGeneratorOptions& options);

    void setupForWideVectors(gfx::Context&, gfx::DrawableBuilder&);

    bool checkAndSetMode(Mode);

    Mode getMode() const { return mode; };

    bool setMode(Mode value) { return mode == value; };

    std::size_t vertexCount() const {
        return std::max(
            {rawVerticesCount, vertices.elements(), polylineVertices.elements(), wideVectorInstanceData.elements()});
    }

    void clear() {
        vertices.clear();
        rawVertices.clear();
        rawVerticesCount = 0;
        polylineVertices.clear();
        polylineIndexes.clear();
        buildIndexes.clear();
        segments.clear();
    }

private:
    LineLayoutVertex layoutVertex(
        Point<int16_t> p, Point<double> e, bool round, bool up, int8_t dir, int32_t linesofar = 0);

    Mode mode{Mode::Custom};
};

} // namespace gfx
} // namespace mbgl
