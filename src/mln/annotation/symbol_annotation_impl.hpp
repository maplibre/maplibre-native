#pragma once

#include <mln/annotation/annotation.hpp>
#include <mln/util/geo.hpp>

#include <string>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace mln {

class AnnotationTileLayer;
class CanonicalTileID;

class SymbolAnnotationImpl {
public:
    SymbolAnnotationImpl(AnnotationID, SymbolAnnotation);

    void updateLayer(const CanonicalTileID&, AnnotationTileLayer&) const;

    const AnnotationID id;
    const SymbolAnnotation annotation;
};

} // namespace mln

namespace boost {
namespace geometry {

// Make Boost Geometry aware of our LatLng type
namespace traits {

template <>
struct tag<mln::LatLng> {
    using type = point_tag;
};
template <>
struct dimension<mln::LatLng> : boost::mpl::int_<2> {};
template <>
struct coordinate_type<mln::LatLng> {
    using type = double;
};
template <>
struct coordinate_system<mln::LatLng> {
    using type = boost::geometry::cs::cartesian;
};

template <>
struct access<mln::LatLng, 0> {
    static inline double get(mln::LatLng const& p) { return p.longitude(); }
};
template <>
struct access<mln::LatLng, 1> {
    static inline double get(mln::LatLng const& p) { return p.latitude(); }
};

template <>
struct tag<mln::LatLngBounds> {
    using type = box_tag;
};
template <>
struct point_type<mln::LatLngBounds> {
    using type = mln::LatLng;
};

template <size_t D>
struct indexed_access<mln::LatLngBounds, min_corner, D> {
    using ct = coordinate_type<mln::LatLng>::type;
    static inline ct get(mln::LatLngBounds const& b) { return geometry::get<D>(b.southwest()); }
    static inline void set(mln::LatLngBounds& b, ct const& value) { geometry::set<D>(b.southwest(), value); }
};

template <size_t D>
struct indexed_access<mln::LatLngBounds, max_corner, D> {
    using ct = coordinate_type<mln::LatLng>::type;
    static inline ct get(mln::LatLngBounds const& b) { return geometry::get<D>(b.northeast()); }
    static inline void set(mln::LatLngBounds& b, ct const& value) { geometry::set<D>(b.northeast(), value); }
};

} // namespace traits

// Tell Boost Geometry how to access a std::shared_ptr<mln::SymbolAnnotation> object.
namespace index {

template <>
struct indexable<std::shared_ptr<const mln::SymbolAnnotationImpl>> {
    using result_type = mln::LatLng;
    mln::LatLng operator()(const std::shared_ptr<const mln::SymbolAnnotationImpl>& v) const {
        const mln::Point<double>& p = v->annotation.geometry;
        return {p.y, p.x};
    }
};

} // namespace index

} // namespace geometry
} // namespace boost
