#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/geometry.hpp>

#include <vector>
#include <memory>
#include <optional>

namespace mbgl {

class TransformState;
class LatLngBounds;

namespace util {

// Helper class to stream tile-cover results per row
class TileCover {
public:
    TileCover(const LatLngBounds&, uint8_t z);
    // When project == true, projects the geometry points to tile coordinates
    TileCover(const Geometry<double>&, uint8_t z, bool project = true);
    ~TileCover();

    std::optional<UnwrappedTileID> next();
    bool hasNext();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

int32_t coveringZoomLevel(double z, style::SourceType type, uint16_t tileSize);

std::vector<OverscaledTileID> tileCover(const TransformState&,
                                        uint8_t z,
                                        const std::optional<uint8_t>& overscaledZ = std::nullopt);
std::vector<UnwrappedTileID> tileCover(const LatLngBounds&, uint8_t z);
std::vector<UnwrappedTileID> tileCover(const Geometry<double>&, uint8_t z);

// Compute only the count of tiles needed for tileCover
uint64_t tileCount(const LatLngBounds&, uint8_t z);
uint64_t tileCount(const Geometry<double>&, uint8_t z);

} // namespace util
} // namespace mbgl
