#include <mln/util/tile_mesh.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/subdivision.hpp>

#include <algorithm>
#include <stdexcept>

namespace mln {
namespace util {

namespace {
constexpr int32_t EXTENT_STENCIL_BORDER = EXTENT / 128;
} // namespace

TileMesh createTileMesh(const TileMeshOptions& options) {
    const int32_t granularity = static_cast<int32_t>(std::max(options.granularity, 1u));

    const int32_t quadsPerAxisX = granularity + (options.generateBorders ? 2 : 0);
    const int32_t quadsPerAxisY = granularity + ((options.extendToNorthPole || options.generateBorders) ? 1 : 0) +
                                  ((options.extendToSouthPole || options.generateBorders) ? 1 : 0);
    const int32_t verticesPerAxisX = quadsPerAxisX + 1;
    const int32_t verticesPerAxisY = quadsPerAxisY + 1;
    const int32_t offsetX = options.generateBorders ? -1 : 0;
    const int32_t offsetY = (options.generateBorders || options.extendToNorthPole) ? -1 : 0;
    const int32_t endX = granularity + (options.generateBorders ? 1 : 0);
    const int32_t endY = granularity + ((options.generateBorders || options.extendToSouthPole) ? 1 : 0);

    if (verticesPerAxisX * verticesPerAxisY > (1 << 16)) {
        throw std::invalid_argument("Tile mesh granularity is too large for 16 bit indices.");
    }

    TileMesh mesh;
    mesh.vertices.reserve(static_cast<std::size_t>(verticesPerAxisX) * verticesPerAxisY * 2);
    for (int32_t y = offsetY; y <= endY; y++) {
        for (int32_t x = offsetX; x <= endX; x++) {
            int32_t vx = x * EXTENT / granularity;
            if (x == -1) {
                vx = -EXTENT_STENCIL_BORDER;
            }
            if (x == granularity + 1) {
                vx = EXTENT + EXTENT_STENCIL_BORDER;
            }
            int32_t vy = y * EXTENT / granularity;
            if (y == -1) {
                vy = options.extendToNorthPole ? NORTH_POLE_Y : -EXTENT_STENCIL_BORDER;
            }
            if (y == granularity + 1) {
                vy = options.extendToSouthPole ? SOUTH_POLE_Y : EXTENT + EXTENT_STENCIL_BORDER;
            }
            mesh.vertices.push_back(static_cast<int16_t>(vx));
            mesh.vertices.push_back(static_cast<int16_t>(vy));
        }
    }

    mesh.indices.reserve(static_cast<std::size_t>(quadsPerAxisX) * quadsPerAxisY * 6);
    for (int32_t y = 0; y < quadsPerAxisY; y++) {
        for (int32_t x = 0; x < quadsPerAxisX; x++) {
            const auto v0 = static_cast<uint16_t>(x + y * verticesPerAxisX);
            const auto v1 = static_cast<uint16_t>((x + 1) + y * verticesPerAxisX);
            const auto v2 = static_cast<uint16_t>(x + (y + 1) * verticesPerAxisX);
            const auto v3 = static_cast<uint16_t>((x + 1) + (y + 1) * verticesPerAxisX);
            mesh.indices.insert(mesh.indices.end(), {v0, v2, v1, v1, v2, v3});
        }
    }
    return mesh;
}

} // namespace util
} // namespace mln
