#pragma once

#include <mbgl/map/mode.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/renderer/buckets/fill_bucket.hpp>

namespace mbgl {

class OverscaledTileID;

using DebugLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

class DebugBucket : private util::noncopyable {
public:
    DebugBucket(const OverscaledTileID& id,
                bool renderable,
                bool complete,
                std::optional<Timestamp> modified,
                std::optional<Timestamp> expires,
                MapDebugOptions);

    void upload(gfx::UploadPass&);

    const bool renderable;
    const bool complete;
    const std::optional<Timestamp> modified;
    const std::optional<Timestamp> expires;
    const MapDebugOptions debugMode;

    gfx::VertexVector<FillLayoutVertex> vertices;
    gfx::IndexVector<gfx::Lines> indices;

    SegmentVector segments;
    SegmentVector tileBorderSegments;
    std::optional<gfx::VertexBuffer<DebugLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> indexBuffer;
};

} // namespace mbgl
