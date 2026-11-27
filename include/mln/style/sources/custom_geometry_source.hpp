#pragma once

#include <mln/style/source.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/geojson.hpp>
#include <mln/util/range.hpp>
#include <mln/util/constants.hpp>

namespace mln {

class OverscaledTileID;
class CanonicalTileID;
template <class T>
class Actor;
class ThreadPool;

namespace style {

using TileFunction = std::function<void(const CanonicalTileID&)>;

class CustomTileLoader;

class CustomGeometrySource final : public Source {
public:
    struct TileOptions {
        double tolerance = 0.375;
        uint16_t tileSize = util::tileSize_I;
        uint16_t buffer = 128;
        bool clip = false;
        bool wrap = false;
    };

    struct Options {
        TileFunction fetchTileFunction;
        TileFunction cancelTileFunction;
        Range<uint8_t> zoomRange = {0, 18};
        TileOptions tileOptions;
    };

public:
    CustomGeometrySource(std::string id, const CustomGeometrySource::Options& options);
    ~CustomGeometrySource() final;
    void loadDescription(FileSource&) final;
    void setTileData(const CanonicalTileID&, const GeoJSON&);
    void invalidateTile(const CanonicalTileID&);
    void invalidateRegion(const LatLngBounds&);
    // Private implementation
    class Impl;
    const Impl& impl() const;
    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<Actor<CustomTileLoader>> loader;
};

template <>
inline bool Source::is<CustomGeometrySource>() const {
    return getType() == SourceType::CustomVector;
}

} // namespace style
} // namespace mln
