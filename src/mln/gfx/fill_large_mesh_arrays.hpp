#pragma once

#include <mln/gfx/index_vector.hpp>
#include <mln/shaders/segment.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace mln {
namespace gfx {

namespace detail {

/// The last segment when the new vertices continue it and fit, otherwise a new one.
inline SegmentBase& prepareSegment(SegmentVector& segments,
                                   std::size_t vertexCount,
                                   std::size_t indexCount,
                                   std::size_t newVertices,
                                   std::size_t maxVertices) {
    if (segments.empty() || segments.back().vertexOffset + segments.back().vertexLength != vertexCount ||
        segments.back().vertexLength + newVertices > maxVertices) {
        segments.emplace_back(vertexCount, indexCount);
    }
    return segments.back();
}

/// Appends the primitives of `Size` vertices in each list, starting a new segment whenever a primitive's missing
/// vertices would not fit the current one; each segment holds copies of the vertices its primitives use.
template <std::size_t Size, typename Vertices, typename LayoutVertex, typename Indexes, typename Lists>
void fillSegments(Vertices& vertices,
                  LayoutVertex& layoutVertex,
                  SegmentVector& segments,
                  Indexes& indexes,
                  std::span<const int16_t> flattened,
                  const Lists& lists,
                  std::size_t maxVertices) {
    constexpr std::size_t unset = std::numeric_limits<std::size_t>::max();
    std::vector<std::size_t> copies(flattened.size() / 2, unset);
    SegmentBase* segment = &prepareSegment(segments, vertices.elements(), indexes.elements(), 0, maxVertices);
    const auto missing = [&](uint32_t index) {
        return copies[index] == unset || copies[index] < segment->vertexOffset;
    };
    for (const auto& primitives : lists) {
        for (std::size_t p = 0; p + Size <= primitives.size(); p += Size) {
            std::size_t needed = 0;
            for (std::size_t k = 0; k < Size; ++k) {
                needed += missing(primitives[p + k]) ? 1 : 0;
            }
            if (segment->vertexLength + needed > maxVertices) {
                segments.emplace_back(vertices.elements(), indexes.elements());
                segment = &segments.back();
            }
            std::array<uint16_t, Size> local{};
            for (std::size_t k = 0; k < Size; ++k) {
                const uint32_t index = primitives[p + k];
                if (missing(index)) {
                    copies[index] = vertices.elements();
                    vertices.emplace_back(layoutVertex(flattened[index * 2], flattened[index * 2 + 1]));
                    segment->vertexLength++;
                }
                local[k] = static_cast<uint16_t>(copies[index] - segment->vertexOffset);
            }
            if constexpr (Size == 3) {
                indexes.emplace_back(local[0], local[1], local[2]);
            } else {
                indexes.emplace_back(local[0], local[1]);
            }
            segment->indexLength += Size;
        }
    }
}

} // namespace detail

/// GL JS's `fillLargeMeshArrays`: appends a triangle mesh, and the outline lines over the same vertices when there
/// are any, to the vertex buffer through `layoutVertex(x, y)`. A mesh that fits one segment goes in whole, its
/// triangles and lines sharing its vertices; a larger one is spread over as many segments as it needs. `maxVertices`
/// is the 16-bit index space unless a test asks for less.
template <typename Vertices, typename LayoutVertex>
void fillLargeMeshArrays(Vertices& vertices,
                         LayoutVertex&& layoutVertex,
                         SegmentVector& triangleSegments,
                         IndexVector<Triangles>& triangleIndexes,
                         std::span<const int16_t> flattened,
                         std::span<const uint32_t> triangleIndices,
                         SegmentVector* lineSegments = nullptr,
                         IndexVector<Lines>* lineIndexes = nullptr,
                         const std::vector<std::vector<uint32_t>>* lineLists = nullptr,
                         std::size_t maxVertices = maxSegmentVertices) {
    const std::size_t vertexCount = flattened.size() / 2;
    const bool hasLines = lineSegments && lineIndexes && lineLists;

    if (vertexCount < maxVertices) {
        auto& triangleSegment = detail::prepareSegment(
            triangleSegments, vertices.elements(), triangleIndexes.elements(), vertexCount, maxVertices);
        const std::size_t triangleBase = triangleSegment.vertexLength;
        SegmentBase* lineSegment =
            hasLines ? &detail::prepareSegment(
                           *lineSegments, vertices.elements(), lineIndexes->elements(), vertexCount, maxVertices)
                     : nullptr;
        const std::size_t lineBase = lineSegment ? lineSegment->vertexLength : 0;

        for (std::size_t i = 0; i + 1 < flattened.size(); i += 2) {
            vertices.emplace_back(layoutVertex(flattened[i], flattened[i + 1]));
        }
        for (std::size_t i = 0; i + 2 < triangleIndices.size(); i += 3) {
            triangleIndexes.emplace_back(static_cast<uint16_t>(triangleBase + triangleIndices[i]),
                                         static_cast<uint16_t>(triangleBase + triangleIndices[i + 1]),
                                         static_cast<uint16_t>(triangleBase + triangleIndices[i + 2]));
        }
        triangleSegment.vertexLength += vertexCount;
        triangleSegment.indexLength += triangleIndices.size();

        if (lineSegment) {
            for (const auto& line : *lineLists) {
                for (std::size_t i = 0; i + 1 < line.size(); i += 2) {
                    lineIndexes->emplace_back(static_cast<uint16_t>(lineBase + line[i]),
                                              static_cast<uint16_t>(lineBase + line[i + 1]));
                }
                lineSegment->indexLength += line.size();
            }
            lineSegment->vertexLength += vertexCount;
        }
        return;
    }

    // Too many vertices for one segment: the triangles, then the lines, each go through segments of their own, copying
    // the vertices they reuse into each segment that needs them.
    detail::fillSegments<3>(vertices,
                            layoutVertex,
                            triangleSegments,
                            triangleIndexes,
                            flattened,
                            std::array<std::span<const uint32_t>, 1>{triangleIndices},
                            maxVertices);
    if (hasLines) {
        detail::fillSegments<2>(
            vertices, layoutVertex, *lineSegments, *lineIndexes, flattened, *lineLists, maxVertices);
    }
}

} // namespace gfx
} // namespace mln
