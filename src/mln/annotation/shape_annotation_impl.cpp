#include <mln/annotation/shape_annotation_impl.hpp>
#include <mln/annotation/annotation_tile.hpp>
#include <mln/annotation/annotation_manager.hpp>
#include <mln/map/world_coordinate_helper.hpp>
#include <mln/tile/tile_id.hpp>
#include <mln/math/wrap.hpp>
#include <mln/math/clamp.hpp>
#include <mln/util/string.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/geojson_reproject.hpp>
#include <mln/util/geometry.hpp>

namespace mln {

using namespace style;

ShapeAnnotationImpl::ShapeAnnotationImpl(const AnnotationID id_)
    : id(id_),
      layerID(AnnotationManager::ShapeLayerID + util::toString(id)) {}

void ShapeAnnotationImpl::updateTileData(const CanonicalTileID& tileID,
                                         AnnotationTileData& data,
                                         const std::shared_ptr<const CRSWorldCoordinateHelper>& projection) {
    static const double baseTolerance = 4;

    if (!shapeTiler || shapeTilerProjection != projection) {
        mapbox::feature::feature_collection<double> features;
        features.emplace_back(ShapeAnnotationGeometry::visit(
            geometry(), [](auto&& geom) { return Feature{std::forward<decltype(geom)>(geom)}; }));
        mapbox::geojsonvt::Options options;
        // The annotation source is currently hard coded to maxzoom 16, so we're
        // topping out at z16 here as well.
        options.maxZoom = 16;
        options.buffer = 255u;
        options.extent = util::EXTENT;
        options.tolerance = baseTolerance;
        if (projection) {
            // A registered projection's world square does not wrap, as with GeoJSON sources.
            options.coordinates = mapbox::geojsonvt::Coordinates::UnitSquare;
            options.wrap = false;
            shapeTiler = std::make_unique<mapbox::geojsonvt::GeoJSONVT>(
                reprojectGeoJSONToWorld(GeoJSON{features}, *projection), options);
        } else {
            shapeTiler = std::make_unique<mapbox::geojsonvt::GeoJSONVT>(features, options);
        }
        shapeTilerProjection = projection;
    }

    const auto& shapeTile = shapeTiler->getTile(tileID.z, tileID.x, tileID.y);
    if (shapeTile.features.empty()) return;

    auto layer = data.addLayer(layerID);

    ToGeometryCollection toGeometryCollection;
    ToFeatureType toFeatureType;
    for (const auto& shapeFeature : shapeTile.features) {
        FeatureType featureType = apply_visitor(toFeatureType, shapeFeature.geometry);
        GeometryCollection renderGeometry = apply_visitor(toGeometryCollection, shapeFeature.geometry);

        assert(featureType != FeatureType::Unknown);

        // https://github.com/mapbox/geojson-vt-cpp/issues/44
        if (featureType == FeatureType::Polygon) {
            renderGeometry = fixupPolygons(renderGeometry);
        }

        layer->addFeature(id, featureType, renderGeometry);
    }
}

} // namespace mln
