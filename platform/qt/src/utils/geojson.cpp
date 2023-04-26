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

Coordinate toCoordinate(const mbgl::Point<double> &point)
{
    return {point.y, point.x};
}

Coordinates toCoordinates(const mbgl::LineString<double> &lineString)
{
    Coordinates coordinates;
    for (auto const &point : lineString)
    {
        coordinates.push_back(toCoordinate(point));
    }
    return coordinates;
}

Coordinates toCoordinates(const mbgl::MultiPoint<double> &points)
{
    Coordinates coordinates;
    for (auto const &point : points)
    {
        coordinates.push_back(toCoordinate(point));
    }
    return coordinates;
}

CoordinatesCollection toCoordinatesCollection(const mbgl::Polygon<double> &polygon)
{
    CoordinatesCollection coordinatesCollection;
    for (auto const &linearRing : polygon)
    {
        Coordinates coordinates;
        for (auto const &point : linearRing)
        {
            coordinates.push_back(toCoordinate(point));
        }
        coordinatesCollection.push_back(coordinates);
    }
    return coordinatesCollection;
}

CoordinatesCollection toCoordinatesCollection(const mbgl::MultiLineString<double> &lineStrings)
{
    CoordinatesCollection coordinatesCollection;
    for (auto const &lineString : lineStrings)
    {
        Coordinates coordinates;
        for (auto const &point : lineString)
        {
            coordinates.push_back(toCoordinate(point));
        }
        coordinatesCollection.push_back(coordinates);
    }
    return coordinatesCollection;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::Point<double> &point)
{
    CoordinatesCollections coordinatesCollections;
    CoordinatesCollection coordinatesCollection;
    Coordinates coordinates;
    coordinates.push_back(toCoordinate(point));
    coordinatesCollection.push_back(coordinates);
    coordinatesCollections.push_back(coordinatesCollection);
    return coordinatesCollections;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::LineString<double> &lineString)
{
    CoordinatesCollections coordinatesCollections;
    CoordinatesCollection coordinatesCollection;
    coordinatesCollection.push_back(toCoordinates(lineString));
    coordinatesCollections.push_back(coordinatesCollection);
    return coordinatesCollections;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::Polygon<double> &polygon)
{
    CoordinatesCollections coordinatesCollections;
    coordinatesCollections.push_back(toCoordinatesCollection(polygon));
    return coordinatesCollections;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::MultiPoint<double> &points)
{
    CoordinatesCollections coordinatesCollections;
    CoordinatesCollection coordinatesCollection;
    coordinatesCollection.push_back(toCoordinates(points));
    coordinatesCollections.push_back(coordinatesCollection);
    return coordinatesCollections;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::MultiLineString<double> &lineStrings)
{
    CoordinatesCollections coordinatesCollections;
    coordinatesCollections.push_back(toCoordinatesCollection(lineStrings));
    return coordinatesCollections;
}

CoordinatesCollections asCoordinatesCollections(const mbgl::MultiPolygon<double> &polygons)
{
    CoordinatesCollections coordinatesCollections;
    for (auto const &polygon : polygons)
    {
        coordinatesCollections.push_back(toCoordinatesCollection(polygon));
    }
    return coordinatesCollections;
}

QVariant asQVariant(const mbgl::Value &value)
{
    auto valueList = [](const mapbox::util::recursive_wrapper<std::vector<mbgl::Value>> &list)
    {
        QVariantList varList;
        varList.reserve(list.get().size());
        for (const auto &listValue : list.get())
        {
            varList.emplace_back(asQVariant(listValue));
        }
        return varList;
    };

    auto valueMap = [](const mapbox::util::recursive_wrapper<std::unordered_map<std::string, mbgl::Value>> &map)
    {
        QVariantMap varMap;
        for (auto it = map.get().begin(); it != map.get().end(); ++it)
        {
            varMap.insert(QString::fromStdString(it->first), asQVariant(it->second));
        }
        return varMap;
    };

    switch (value.which())
    {
    // Null
    case 0:
        return {};
    // Bool
    case 1:
        return {value.get<bool>()};
    // uint64_t
    case 2:
        return {value.get<uint64_t>()};
    // int64_t
    case 3:
        return {value.get<int64_t>()};
    // double
    case 4:
        return {value.get<double>()};
    // std::string
    case 5:
        return QString::fromStdString(value.get<std::string>());
    // mapbox::util::recursive_wrapper<std::vector<value>>
    case 6:
        return valueList(value.get<mapbox::util::recursive_wrapper<std::vector<mbgl::Value>>>());
    // mapbox::util::recursive_wrapper<std::unordered_map<std::string, value>>
    case 7:
        return valueMap(value.get<mapbox::util::recursive_wrapper<std::unordered_map<std::string, mbgl::Value>>>());
    default:
        qWarning();
        return {};
    }
}

QVariant asQVariant(const mbgl::FeatureIdentifier &id)
{
    switch (id.which())
    {
    // Null
    case 0:
        return {};
    // uint64_t
    case 1:
        return {id.get<uint64_t>()};
    // int64_t
    case 2:
        return {id.get<int64_t>()};
    // double
    case 3:
        return {id.get<double>()};
    // std::string
    case 4:
        return QString::fromStdString(id.get<std::string>());
    default:
        qWarning();
        return {};
    }
}

Feature asFeature(const mbgl::GeoJSONFeature &feature)
{
    QVariantMap properties;
    for (auto it = feature.properties.begin(); it != feature.properties.end(); it++)
    {
        properties.insert(QString::fromStdString(it->first), asQVariant(it->second));
    }

    auto id = asQVariant(feature.id);

    // auto geometry = asCoordinatesCollections(feature.geometry);
    switch (feature.geometry.which())
    {
    case 0:
        return {Feature::PointType, CoordinatesCollections(), properties, id};
    case 1:
        return {Feature::PointType, asCoordinatesCollections(feature.geometry.get<mbgl::Point<double>>()), std::move(properties), std::move(id)};
    case 2:
        return {Feature::LineStringType, asCoordinatesCollections(feature.geometry.get<mbgl::LineString<double>>()), std::move(properties), std::move(id)};
    case 3:
        return {Feature::PolygonType, asCoordinatesCollections(feature.geometry.get<mbgl::Polygon<double>>()), std::move(properties), std::move(id)};
    case 4:
        return {Feature::PointType, asCoordinatesCollections(feature.geometry.get<mbgl::MultiPoint<double>>()), std::move(properties), std::move(id)};
    case 5:
        return {Feature::LineStringType, asCoordinatesCollections(feature.geometry.get<mbgl::MultiLineString<double>>()), std::move(properties), std::move(id)};
    case 6:
        return {Feature::PolygonType, asCoordinatesCollections(feature.geometry.get<mbgl::MultiPolygon<double>>()), std::move(properties), std::move(id)};
    default:
        return {};
    }
}
} // namespace QMapLibreGL::GeoJSON
