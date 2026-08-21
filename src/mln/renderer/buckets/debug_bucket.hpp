#pragma once

#include <mln/map/mode.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/geometry.hpp>
#include <mln/util/noncopyable.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/gfx/index_buffer.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/buckets/fill_bucket.hpp>

namespace mln {

class OverscaledTileID;

using DebugLayoutVertex = gfx::Vertex<TypeList<attributes::pos>>;

class DebugBucket : private util::noncopyable {
public:
    DebugBucket(const OverscaledTileID& id,
                bool renderable,
                bool complete,
                std::optional<Timestamp> modified,
                std::optional<Timestamp> expires,
                MapDebugOptions,
                std::string sourceName);

    void upload(gfx::UploadPass&);

    const bool renderable;
    const bool complete;
    const std::optional<Timestamp> modified;
    const std::optional<Timestamp> expires;
    const MapDebugOptions debugMode;
    const std::string sourceName;

    gfx::VertexVector<FillLayoutVertex> vertices;
    gfx::IndexVector<gfx::Lines> indices;

    SegmentVector segments;
    SegmentVector tileBorderSegments;
    std::optional<gfx::VertexBuffer<DebugLayoutVertex>> vertexBuffer;
    std::optional<gfx::IndexBuffer> indexBuffer;
};

} // namespace mln
