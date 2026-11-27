#pragma once

#include <mln/style/projection.hpp>
#include <mln/style/source.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/geojson.hpp>
#include <mln/util/range.hpp>
#include <mln/util/constants.hpp>

#include <mutex>
#include <optional>

namespace mln {

class OverscaledTileID;
class CanonicalTileID;
template <class T>
class Actor;
class ThreadPool;

namespace style {

using TileFunction = std::function<void(const CanonicalTileID&)>;

class CustomTileLoader;
class Style;

// NOTE: Any derived class must invalidate `weakFactory` in the destructor
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
    /// The lat/lng box of a tile in the projection of the style the source is in, for a fetch callback on any thread.
    /// In a registered projection it is the box of the tile's four corners, as in GL JS, which can miss part of the
    /// tile, such as a pole inside it.
    LatLngBounds getTileBounds(const CanonicalTileID&) const;
    // Private implementation
    class Impl;
    const Impl& impl() const;
    bool supportsLayerType(const mln::style::LayerTypeInfo*) const override;
    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    friend class Style;
    /// The style calls it when the source joins it and when its projection changes.
    void setProjection(Immutable<Projection::Impl>);

    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<Actor<CustomTileLoader>> loader;
    mutable std::mutex projectionMutex;
    std::optional<Immutable<Projection::Impl>> projection;
    mapbox::base::WeakPtrFactory<Source> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`
};

template <>
inline bool Source::is<CustomGeometrySource>() const {
    return getType() == SourceType::CustomVector;
}

} // namespace style
} // namespace mln
