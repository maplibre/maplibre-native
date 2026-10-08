#include <mln/test/util.hpp>

#include <mln/gfx/fill_large_mesh_arrays.hpp>

#include <array>
#include <cstdint>
#include <vector>

using namespace mln;

namespace {

struct Mesh {
    std::vector<int16_t> vertices;
    std::vector<uint32_t> triangles;
    std::vector<uint32_t> lines;
};

// GL JS's `getGridMesh`: `size` by `size` quads, with the grid's outline as lines.
Mesh gridMesh(uint32_t size) {
    Mesh mesh;
    const uint32_t verticesPerAxis = size + 1;
    for (uint32_t y = 0; y < verticesPerAxis; ++y) {
        for (uint32_t x = 0; x < verticesPerAxis; ++x) {
            mesh.vertices.push_back(static_cast<int16_t>(x));
            mesh.vertices.push_back(static_cast<int16_t>(y));
        }
    }
    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            const uint32_t i00 = y * verticesPerAxis + x;
            const uint32_t i10 = y * verticesPerAxis + x + 1;
            const uint32_t i01 = (y + 1) * verticesPerAxis + x;
            const uint32_t i11 = (y + 1) * verticesPerAxis + x + 1;
            mesh.triangles.insert(mesh.triangles.end(), {i00, i11, i10, i00, i01, i11});
        }
    }
    for (uint32_t i = 0; i < size; ++i) {
        mesh.lines.insert(mesh.lines.end(), {i, i + 1});
    }
    for (uint32_t i = 0; i < size; ++i) {
        mesh.lines.insert(mesh.lines.end(), {verticesPerAxis * size + i, verticesPerAxis * size + i + 1});
    }
    for (uint32_t i = 0; i < size; ++i) {
        mesh.lines.insert(mesh.lines.end(), {i * verticesPerAxis, (i + 1) * verticesPerAxis});
    }
    for (uint32_t i = 0; i < size; ++i) {
        mesh.lines.insert(mesh.lines.end(), {i * verticesPerAxis + size, (i + 1) * verticesPerAxis + size});
    }
    return mesh;
}

// GL JS's `mergeMeshes`: the meshes as one, which is what filling them one after another must draw.
Mesh merge(const std::vector<Mesh>& meshes) {
    Mesh merged;
    for (const auto& mesh : meshes) {
        const auto base = static_cast<uint32_t>(merged.vertices.size() / 2);
        merged.vertices.insert(merged.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        for (const uint32_t index : mesh.triangles) {
            merged.triangles.push_back(base + index);
        }
        for (const uint32_t index : mesh.lines) {
            merged.lines.push_back(base + index);
        }
    }
    return merged;
}

struct Vertices {
    std::vector<std::array<int16_t, 2>> positions;
    std::size_t elements() const { return positions.size(); }
    void emplace_back(std::array<int16_t, 2> position) { positions.push_back(position); }
};

struct Buffers {
    Vertices vertices;
    SegmentVector triangleSegments;
    gfx::IndexVector<gfx::Triangles> triangles;
    SegmentVector lineSegments;
    gfx::IndexVector<gfx::Lines> lines;
};

void fill(Buffers& buffers, const Mesh& mesh, std::size_t maxVertices) {
    const std::vector<std::vector<uint32_t>> lineLists{mesh.lines};
    gfx::fillLargeMeshArrays(
        buffers.vertices,
        [](int16_t x, int16_t y) { return std::array<int16_t, 2>{x, y}; },
        buffers.triangleSegments,
        buffers.triangles,
        mesh.vertices,
        mesh.triangles,
        &buffers.lineSegments,
        &buffers.lines,
        &lineLists,
        maxVertices);
}

// GL JS's `getRenderedGeometryRepresentation`: the primitives a draw of every segment fetches, in order.
template <std::size_t Size, typename Indexes>
std::vector<std::array<int16_t, Size * 2>> drawn(const Buffers& buffers,
                                                 const SegmentVector& segments,
                                                 const Indexes& indexes,
                                                 std::size_t maxVertices) {
    std::vector<std::array<int16_t, Size * 2>> primitives;
    for (const auto& segment : segments) {
        EXPECT_LE(segment.vertexLength, maxVertices);
        for (std::size_t i = segment.indexOffset; i + Size <= segment.indexOffset + segment.indexLength; i += Size) {
            std::array<int16_t, Size * 2> primitive{};
            for (std::size_t k = 0; k < Size; ++k) {
                const uint16_t index = indexes.at(i + k);
                EXPECT_LT(index, segment.vertexLength);
                primitive[k * 2] = buffers.vertices.positions.at(segment.vertexOffset + index)[0];
                primitive[k * 2 + 1] = buffers.vertices.positions.at(segment.vertexOffset + index)[1];
            }
            primitives.push_back(primitive);
        }
    }
    return primitives;
}

template <std::size_t Size>
std::vector<std::array<int16_t, Size * 2>> drawn(const Mesh& mesh, const std::vector<uint32_t>& indexes) {
    std::vector<std::array<int16_t, Size * 2>> primitives;
    for (std::size_t i = 0; i + Size <= indexes.size(); i += Size) {
        std::array<int16_t, Size * 2> primitive{};
        for (std::size_t k = 0; k < Size; ++k) {
            primitive[k * 2] = mesh.vertices[indexes[i + k] * 2];
            primitive[k * 2 + 1] = mesh.vertices[indexes[i + k] * 2 + 1];
        }
        primitives.push_back(primitive);
    }
    return primitives;
}

// GL JS's `testMeshesEqual`: the buffers draw the same triangles and lines as the mesh, in the same order.
void expectDrawsTheSame(const Mesh& mesh, const Buffers& buffers, std::size_t maxVertices) {
    EXPECT_EQ(drawn<3>(mesh, mesh.triangles),
              drawn<3>(buffers, buffers.triangleSegments, buffers.triangles, maxVertices));
    EXPECT_EQ(drawn<2>(mesh, mesh.lines), drawn<2>(buffers, buffers.lineSegments, buffers.lines, maxVertices));
}

} // namespace

TEST(FillLargeMeshArrays, TinyMeshIsUnchanged) {
    const Mesh mesh = gridMesh(1);
    Buffers buffers;
    fill(buffers, mesh, 16);
    EXPECT_EQ(1u, buffers.triangleSegments.size());
    expectDrawsTheSame(mesh, buffers, 16);
}

TEST(FillLargeMeshArrays, SmallMeshIsUnchanged) {
    const Mesh mesh = gridMesh(2);
    Buffers buffers;
    fill(buffers, mesh, 16);
    EXPECT_EQ(1u, buffers.triangleSegments.size());
    expectDrawsTheSame(mesh, buffers, 16);
}

TEST(FillLargeMeshArrays, LargeMeshIsSplitIntoSeveralSegments) {
    const Mesh mesh = gridMesh(4);
    Buffers buffers;
    fill(buffers, mesh, 16);
    EXPECT_GT(buffers.triangleSegments.size(), 1u);
    expectDrawsTheSame(mesh, buffers, 16);
}

TEST(FillLargeMeshArrays, VeryLargeMeshIsSplitIntoSeveralSegments) {
    const Mesh mesh = gridMesh(64);
    Buffers buffers;
    fill(buffers, mesh, 1024);
    EXPECT_GT(buffers.triangleSegments.size(), 1u);
    expectDrawsTheSame(mesh, buffers, 1024);
}

TEST(FillLargeMeshArrays, SeveralSmallMeshesShareOneSegment) {
    const Mesh small = gridMesh(1);
    Buffers buffers;
    fill(buffers, small, 16);
    fill(buffers, small, 16);
    std::vector<std::array<int16_t, 2>> vertices{{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 0}, {1, 0}, {0, 1}, {1, 1}};
    EXPECT_EQ(vertices, buffers.vertices.positions);
    EXPECT_EQ(std::vector<uint16_t>({0, 3, 1, 0, 2, 3, 4, 7, 5, 4, 6, 7}), buffers.triangles.vector());
    EXPECT_EQ(std::vector<uint16_t>({0, 1, 2, 3, 0, 2, 1, 3, 4, 5, 6, 7, 4, 6, 5, 7}), buffers.lines.vector());
    EXPECT_EQ(1u, buffers.triangleSegments.size());
    EXPECT_EQ(1u, buffers.lineSegments.size());
}

TEST(FillLargeMeshArrays, MeshesStartANewSegmentWhenTheyNoLongerFit) {
    const Mesh small = gridMesh(1); // 4 vertices
    const Mesh large = gridMesh(2); // 9 vertices
    // The first two share a segment (13 vertices); the third would make it 22, so it starts the second, which the
    // fourth then shares.
    const std::vector<Mesh> meshes{small, large, large, small};
    Buffers buffers;
    for (const auto& mesh : meshes) {
        fill(buffers, mesh, 16);
    }
    ASSERT_EQ(2u, buffers.triangleSegments.size());
    EXPECT_EQ(30u, buffers.triangleSegments[0].indexLength);
    EXPECT_EQ(30u, buffers.triangleSegments[1].indexLength);
    expectDrawsTheSame(merge(meshes), buffers, 16);
}

TEST(FillLargeMeshArrays, ManySmallAndLargeMeshes) {
    const Mesh small = gridMesh(1);
    const Mesh large = gridMesh(2);
    const std::vector<Mesh> meshes{small, large, large, small, small, small, large, large, large, large, large};
    Buffers buffers;
    for (const auto& mesh : meshes) {
        fill(buffers, mesh, 16);
    }
    const Mesh merged = merge(meshes);
    expectDrawsTheSame(merged, buffers, 16);
    EXPECT_GT(buffers.triangleSegments.size(), merged.vertices.size() / 2 / 16);
    EXPECT_LT(buffers.triangleSegments.size(), meshes.size());
}

TEST(FillLargeMeshArrays, MeshesAfterASplitStartAFreshSegment) {
    // A split mesh copies its triangles' vertices and then its lines' after them, so the last triangle segment no
    // longer ends where the buffer does: the next mesh must not continue it.
    const Mesh large = gridMesh(4);
    const Mesh small = gridMesh(1);
    Buffers buffers;
    fill(buffers, large, 16);
    fill(buffers, small, 16);
    expectDrawsTheSame(merge({large, small}), buffers, 16);
}
