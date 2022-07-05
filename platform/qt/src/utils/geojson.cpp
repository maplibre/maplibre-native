#include "geojson.hpp"

#include <QDebug>

namespace QMapLibreGL::GeoJSON {

mbgl::Point<double> asPoint(const Coordinate &coordinate) {
    return mbgl::Point<double> { coordinate.second, coordinate.first };
}

mbgl::MultiPoint<double> asMultiPoint(const Coordinates &multiPoint) {
    mbgl::MultiPoint<double> mbglMultiPoint;
    mbglMultiPoint.reserve(multiPoint.size());
    for (const auto &point: multiPoint) {
        mbglMultiPoint.emplace_back(asPoint(point));
    }
    return mbglMultiPoint;
};

mbgl::LineString<double> asLineString(const Coordinates &lineString) {
    mbgl::LineString<double> mbglLineString;
    mbglLineString.reserve(lineString.size());
    for (const auto &coordinate : lineString) {
        mbglLineString.emplace_back(asPoint(coordinate));
    }
    return mbglLineString;
};

mbgl::MultiLineString<double> asMultiLineString(const CoordinatesCollection &multiLineString) {
    mbgl::MultiLineString<double> mbglMultiLineString;
    mbglMultiLineString.reserve(multiLineString.size());
    for (const auto &lineString : multiLineString) {
        mbglMultiLineString.emplace_back(std::forward<mbgl::LineString<double>>(asLineString(lineString)));
    }
    return mbglMultiLineString;
};

mbgl::Polygon<double> asPolygon(const CoordinatesCollection &polygon) {
    mbgl::Polygon<double> mbglPolygon;
    mbglPolygon.reserve(polygon.size());
    for (const auto &linearRing : polygon) {
        mbgl::LinearRing<double> mbglLinearRing;
        mbglLinearRing.reserve(linearRing.size());
        for (const auto &coordinate: linearRing) {
            mbglLinearRing.emplace_back(asPoint(coordinate));
        }
        mbglPolygon.emplace_back(std::move(mbglLinearRing));
    }
    return mbglPolygon;
};

mbgl::MultiPolygon<double> asMultiPolygon(const CoordinatesCollections &multiPolygon) {
    mbgl::MultiPolygon<double> mbglMultiPolygon;
    mbglMultiPolygon.reserve(multiPolygon.size());
    for (const auto &polygon : multiPolygon) {
        mbglMultiPolygon.emplace_back(std::forward<mbgl::Polygon<double>>(asPolygon(polygon)));
    }
    return mbglMultiPolygon;
};

mbgl::Value asPropertyValue(const QVariant &value) {
    auto valueList = [](const QVariantList &list) {
        std::vector<mbgl::Value> mbglList;
        mbglList.reserve(list.size());
        for (const auto& listValue : list) {
            mbglList.emplace_back(asPropertyValue(listValue));
        }
        return mbglList;
    };

    auto valueMap = [](const QVariantMap &map) {
        std::unordered_map<std::string, mbgl::Value> mbglMap;
        mbglMap.reserve(map.size());
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            mbglMap.emplace(std::make_pair(it.key().toStdString(), asPropertyValue(it.value())));
        }
        return mbglMap;
    };

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    switch (value.typeId()) {
#else
    switch (static_cast<QMetaType::Type>(value.type())) {
#endif
    case QMetaType::UnknownType:
        return mbgl::NullValue {};
    case QMetaType::Bool:
        return { value.toBool() };
    case QMetaType::ULongLong:
        return { uint64_t(value.toULongLong()) };
    case QMetaType::LongLong:
        return { int64_t(value.toLongLong()) };
    case QMetaType::Double:
        return { value.toDouble() };
    case QMetaType::QString:
        return { value.toString().toStdString() };
    case QMetaType::QVariantList:
        return valueList(value.toList());
    case QMetaType::QVariantMap:
        return valueMap(value.toMap());
    default:
        qWarning() << "Unsupported feature property value:" << value;
        return {};
    }
}

mbgl::FeatureIdentifier asFeatureIdentifier(const QVariant &id) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    switch (id.typeId()) {
#else
    switch (static_cast<QMetaType::Type>(id.type())) {
#endif
    case QMetaType::UnknownType:
        return {};
    case QMetaType::ULongLong:
        return { uint64_t(id.toULongLong()) };
    case QMetaType::LongLong:
        return { int64_t(id.toLongLong()) };
    case QMetaType::Double:
        return { id.toDouble() };
    case QMetaType::QString:
        return { id.toString().toStdString() };
    default:
        qWarning() << "Unsupported feature identifier:" << id;
        return {};
    }
}

mbgl::GeoJSONFeature asFeature(const Feature &feature) {
    mbgl::PropertyMap properties;
    properties.reserve(feature.properties.size());
    for (auto it = feature.properties.constBegin(); it != feature.properties.constEnd(); ++it) {
        properties.emplace(std::make_pair(it.key().toStdString(), asPropertyValue(it.value())));
    }

    mbgl::FeatureIdentifier id = asFeatureIdentifier(feature.id);

    if (feature.type == Feature::PointType) {
        const Coordinates &points = feature.geometry.first().first();
        if (points.size() == 1) {
            return { asPoint(points.first()), std::move(properties), std::move(id) };
        } else {
            return { asMultiPoint(points), std::move(properties), std::move(id) };
        }
    } else if (feature.type == Feature::LineStringType) {
        const CoordinatesCollection &lineStrings = feature.geometry.first();
        if (lineStrings.size() == 1) {
            return { asLineString(lineStrings.first()), std::move(properties), std::move(id) };
        } else {
            return { asMultiLineString(lineStrings), std::move(properties), std::move(id) };
        }
    } else { // PolygonType
        const CoordinatesCollections &polygons = feature.geometry;
        if (polygons.size() == 1) {
            return { asPolygon(polygons.first()), std::move(properties), std::move(id) };
        } else {
            return { asMultiPolygon(polygons), std::move(properties), std::move(id) };
        }
    }
};

} // namespace QMapLibreGL::GeoJSON
