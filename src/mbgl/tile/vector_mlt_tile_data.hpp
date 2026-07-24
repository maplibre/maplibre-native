#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>

#include <memory>
#include <string>
#include <vector>

namespace mbgl {

class VectorMLTTileData : public GeometryTileData {
public:
    VectorMLTTileData(std::shared_ptr<const std::string> data, bool fastPFOREnabled);
    VectorMLTTileData(const VectorMLTTileData&);
    VectorMLTTileData(VectorMLTTileData&&) noexcept;
    ~VectorMLTTileData() override;

    std::unique_ptr<GeometryTileData> clone() const override;
    std::unique_ptr<GeometryTileLayer> getLayer(const std::string& name) const override;

    std::vector<std::string> layerNames() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
