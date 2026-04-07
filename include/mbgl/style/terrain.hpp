#pragma once

#include <mbgl/util/immutable.hpp>
#include <string>

namespace mbgl {
namespace style {

class TerrainObserver;

/**
 * @brief Terrain configuration for 3D terrain rendering
 *
 * Terrain allows the map to be draped over digital elevation model (DEM) data.
 * It requires a raster-dem source and provides an optional exaggeration multiplier.
 */
class Terrain {
public:
    Terrain();
    explicit Terrain(const std::string& sourceID, float exaggeration = 1.0f);
    ~Terrain();

    /**
     * @brief Get the ID of the raster-dem source providing elevation data
     */
    std::string getSource() const;

    /**
     * @brief Set the ID of the raster-dem source providing elevation data
     */
    void setSource(const std::string& sourceID);

    /**
     * @brief Get the elevation exaggeration multiplier
     * @return Exaggeration factor (default: 1.0)
     */
    float getExaggeration() const;

    /**
     * @brief Set the elevation exaggeration multiplier
     * @param exaggeration Multiplier for elevation values (e.g., 1.5 = 50% more dramatic)
     */
    void setExaggeration(float exaggeration);

    // Internal implementation
    class Impl;
    Immutable<Impl> impl;
    explicit Terrain(Immutable<Impl>);
    Mutable<Impl> mutableImpl() const;

    TerrainObserver* observer = nullptr;
    void setObserver(TerrainObserver*);
};

} // namespace style
} // namespace mbgl
