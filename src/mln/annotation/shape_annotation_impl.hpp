#pragma once

#include <mln/util/string.hpp>
#include <mapbox/geojsonvt.hpp>

#include <mln/annotation/annotation.hpp>
#include <mln/util/geometry.hpp>
#include <mln/style/style.hpp>

#include <string>
#include <memory>

namespace mln {

class AnnotationTileData;
class CanonicalTileID;

class ShapeAnnotationImpl {
public:
    ShapeAnnotationImpl(AnnotationID);
    virtual ~ShapeAnnotationImpl() = default;

    virtual void updateStyle(style::Style::Impl &) const = 0;
    virtual const ShapeAnnotationGeometry &geometry() const = 0;

    void updateTileData(const CanonicalTileID &, AnnotationTileData &);

    const AnnotationID id;
    const std::string layerID;
    std::unique_ptr<mapbox::geojsonvt::GeoJSONVT> shapeTiler;
};

struct CloseShapeAnnotation {
    ShapeAnnotationGeometry operator()(const mln::LineString<double> &geom) const { return geom; }
    ShapeAnnotationGeometry operator()(const mln::MultiLineString<double> &geom) const { return geom; }
    ShapeAnnotationGeometry operator()(const mln::Polygon<double> &geom) const {
        mln::Polygon<double> closed = geom;
        for (auto &ring : closed) {
            if (!ring.empty() && ring.front() != ring.back()) {
                ring.emplace_back(ring.front());
            }
        }
        return closed;
    }
    ShapeAnnotationGeometry operator()(const mln::MultiPolygon<double> &geom) const {
        mln::MultiPolygon<double> closed = geom;
        for (auto &polygon : closed) {
            for (auto &ring : polygon) {
                if (!ring.empty() && ring.front() != ring.back()) {
                    ring.emplace_back(ring.front());
                }
            }
        }
        return closed;
    }
};

} // namespace mln
