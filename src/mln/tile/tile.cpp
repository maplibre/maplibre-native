#include <mln/tile/tile.hpp>
#include <mln/tile/tile_observer.hpp>
#include <mln/renderer/query.hpp>
#include <mln/util/string.hpp>
#include <mln/util/logging.hpp>

namespace mln {

namespace {
TileObserver nullObserver;
}

Tile::Tile(Kind kind_, OverscaledTileID id_, std::string sourceID_, TileObserver* observer_)
    : kind(kind_),
      id(id_),
      sourceID(std::move(sourceID_)) {
    observer = observer_ ? observer_ : &nullObserver;
}

Tile::~Tile() = default;

void Tile::setObserver(TileObserver* observer_) {
    observer = observer_;
}

void Tile::setTriedCache() {
    triedOptional = true;
    observer->onTileChanged(*this);
}

void Tile::dumpDebugLogs() const {
    std::string kindString;
    switch (kind) {
        case Kind::Geometry:
            kindString = "Geometry";
            break;
        case Kind::Raster:
            kindString = "Raster";
            break;
        case Kind::RasterDEM:
            kindString = "RasterDEM";
            break;
        default:
            kindString = "Unknown";
            break;
    }
    Log::Info(Event::General, "Tile::Kind: " + kindString);
    Log::Info(Event::General, "Tile::id: " + util::toString(id));
    Log::Info(Event::General, "Tile::renderable: " + std::string(isRenderable() ? "yes" : "no"));
    Log::Info(Event::General, "Tile::complete: " + std::string(isComplete() ? "yes" : "no"));
    Log::Info(Event::General, "Tile::loaded: " + std::string(isLoaded() ? "yes" : "no"));
}

void Tile::queryRenderedFeatures(std::unordered_map<std::string, std::vector<Feature>>&,
                                 const GeometryCoordinates&,
                                 const TransformState&,
                                 const std::unordered_map<std::string, const RenderLayer*>&,
                                 const RenderedQueryOptions&,
                                 const mat4&,
                                 const SourceFeatureState&) {}

float Tile::getQueryPadding(const std::unordered_map<std::string, const RenderLayer*>&) {
    return 0;
}

void Tile::querySourceFeatures(std::vector<Feature>&, const SourceQueryOptions&) {}

void Tile::onTileAction(TileOperation op) {
    observer->onTileAction(id, sourceID, op);
};

} // namespace mln
