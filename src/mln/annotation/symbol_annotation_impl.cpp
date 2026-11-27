#include <mln/annotation/symbol_annotation_impl.hpp>
#include <mln/annotation/annotation_tile.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/math/clamp.hpp>
#include <mln/util/tile_coordinate.hpp>

namespace mln {

SymbolAnnotationImpl::SymbolAnnotationImpl(AnnotationID id_, SymbolAnnotation annotation_)
    : id(id_),
      annotation(std::move(annotation_)) {}

void SymbolAnnotationImpl::updateLayer(const CanonicalTileID& tileID, AnnotationTileLayer& layer) const {
    LatLng latLng{annotation.geometry.y, annotation.geometry.x};
    updateLayer(tileID, layer, TileCoordinate::fromLatLng(0, latLng).p);
}

void SymbolAnnotationImpl::updateLayer(const CanonicalTileID& tileID,
                                       AnnotationTileLayer& layer,
                                       const Point<double>& world) const {
    std::unordered_map<std::string, std::string> featureProperties;
    featureProperties.emplace("sprite", annotation.icon.empty() ? std::string("default_marker") : annotation.icon);

    GeometryCoordinate tilePoint = TileCoordinate::toGeometryCoordinate(UnwrappedTileID(0, tileID), world);
    layer.addFeature(id, FeatureType::Point, GeometryCollection{{{{tilePoint}}}}, featureProperties);
}

} // namespace mln
