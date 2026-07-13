#include <mbgl/renderer/render_source.hpp>

#include <mbgl/annotation/render_annotation_source.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/renderer/render_source_observer.hpp>
#include <mbgl/renderer/sources/render_geojson_source.hpp>
#include <mbgl/renderer/sources/render_raster_source.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/sources/render_vector_source.hpp>
#include <mbgl/renderer/sources/render_image_source.hpp>
#include <mbgl/renderer/sources/render_custom_geometry_source.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/constants.hpp>

#include <memory>
#include <utility>

namespace mbgl {

using namespace style;

std::unique_ptr<RenderSource> RenderSource::create(const Immutable<Source::Impl>& impl,
                                                   const TaggedScheduler& threadPool_) {
    switch (impl->type) {
        case SourceType::Vector:
            // the TileSet isn't available yet, so we can't make different sources by format
            return std::make_unique<RenderVectorSource>(staticImmutableCast<TileSource::Impl>(impl),
                                                        std::move(threadPool_));
        case SourceType::Raster:
            return std::make_unique<RenderRasterSource>(staticImmutableCast<TileSource::Impl>(impl),
                                                        std::move(threadPool_));
        case SourceType::RasterDEM:
            return std::make_unique<RenderRasterDEMSource>(staticImmutableCast<TileSource::Impl>(impl),
                                                           std::move(threadPool_));
        case SourceType::GeoJSON:
            return std::make_unique<RenderGeoJSONSource>(staticImmutableCast<GeoJSONSource::Impl>(impl),
                                                         std::move(threadPool_));
        case SourceType::Video:
            assert(false);
            return nullptr;
        case SourceType::Annotations:
            if (LayerManager::annotationsEnabled) {
                return std::make_unique<RenderAnnotationSource>(staticImmutableCast<AnnotationSource::Impl>(impl),
                                                                std::move(threadPool_));
            } else {
                assert(false);
                return nullptr;
            }
        case SourceType::Image:
            return std::make_unique<RenderImageSource>(staticImmutableCast<ImageSource::Impl>(impl));
        case SourceType::CustomVector:
            return std::make_unique<RenderCustomGeometrySource>(staticImmutableCast<CustomGeometrySource::Impl>(impl),
                                                                std::move(threadPool_));
    }

    // Not reachable, but placate GCC.
    assert(false);
    return nullptr;
}

namespace {
RenderSourceObserver nullObserver;
}

RenderSource::RenderSource(Immutable<style::Source::Impl> impl)
    : baseImpl(std::move(impl)),
      observer(&nullObserver) {}

RenderSource::~RenderSource() = default;

void RenderSource::setObserver(RenderSourceObserver* observer_) {
    observer = observer_;
}

void RenderSource::onTileChanged(Tile& tile) {
    observer->onTileChanged(*this, tile.id);
}

void RenderSource::onTileError(Tile& tile, std::exception_ptr error) {
    observer->onTileError(*this, tile.id, error);
}

void RenderSource::onTileAction(OverscaledTileID id, std::string sourceID, TileOperation op) {
    observer->onTileAction(*this, op, id, sourceID);
}

bool RenderSource::isEnabled() const {
    return enabled;
}

uint8_t RenderSource::getMaxZoom() const {
    assert(false);
    return util::TERRAIN_RGB_MAXZOOM;
}

Immutable<std::vector<RenderTile>> RenderSource::getRawRenderTiles() const {
    return makeMutable<std::vector<RenderTile>>();
}

} // namespace mbgl
