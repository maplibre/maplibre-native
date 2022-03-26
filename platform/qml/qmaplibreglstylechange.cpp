/****************************************************************************
**
** Copyright (C) 2017 Mapbox, Inc.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmaplibreglstylechange_p.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaProperty>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>
#include <QtQml/QJSValue>
#include <QtLocation/private/qdeclarativecirclemapitem_p_p.h>

namespace {

QByteArray formatPropertyName(const QByteArray &name)
{
    QString nameAsString = QString::fromLatin1(name);
    static const QRegularExpression camelCaseRegex(QStringLiteral("([a-z0-9])([A-Z])"));
    return nameAsString.replace(camelCaseRegex, QStringLiteral("\\1-\\2")).toLower().toLatin1();
}

bool isImmutableProperty(const QByteArray &name)
{
    return name == QStringLiteral("type") || name == QStringLiteral("layer");
}

QString getId(QDeclarativeGeoMapItemBase *mapItem)
{
    return QStringLiteral("QtLocation-") +
            ((mapItem->objectName().isEmpty()) ? QString::number(quint64(mapItem)) : mapItem->objectName());
}

// Mapbox GL supports geometry segments that spans above 180 degrees in
// longitude. To keep visual expectations in parity with Qt, we need to adapt
// the coordinates to always use the shortest path when in ambiguity.
static bool geoRectangleCrossesDateLine(const QGeoRectangle &rect) {
    return rect.topLeft().longitude() > rect.bottomRight().longitude();
}

QMapbox::Feature featureFromMapRectangle(QDeclarativeRectangleMapItem *mapItem)
{
    const QGeoRectangle *rect = static_cast<const QGeoRectangle *>(&mapItem->geoShape());
    QMapbox::Coordinate bottomLeft { rect->bottomLeft().latitude(), rect->bottomLeft().longitude() };
    QMapbox::Coordinate topLeft { rect->topLeft().latitude(), rect->topLeft().longitude() };
    QMapbox::Coordinate bottomRight { rect->bottomRight().latitude(), rect->bottomRight().longitude() };
    QMapbox::Coordinate topRight { rect->topRight().latitude(), rect->topRight().longitude() };
    if (geoRectangleCrossesDateLine(*rect)) {
        bottomRight.second += 360.0;
        topRight.second += 360.0;
    }
    QMapbox::CoordinatesCollections geometry { { { bottomLeft, bottomRight, topRight, topLeft, bottomLeft } } };

    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapCircle(QDeclarativeCircleMapItem *mapItem)
{
    static const int circleSamples = 128;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(mapItem->map()->geoProjection());
    QList<QGeoCoordinate> path;
    QGeoCoordinate leftBound;
    QDeclarativeCircleMapItemPrivateCPU::calculatePeripheralPoints(path, mapItem->center(), mapItem->radius(), circleSamples, leftBound);
    QList<QDoubleVector2D> pathProjected;
    for (const QGeoCoordinate &c : qAsConst(path))
        pathProjected << p.geoToMapProjection(c);
    if (QDeclarativeCircleMapItemPrivateCPU::crossEarthPole(mapItem->center(), mapItem->radius()))
        QDeclarativeCircleMapItemPrivateCPU::preserveCircleGeometry(pathProjected, mapItem->center(), mapItem->radius(), p);
    path.clear();
    for (const QDoubleVector2D &c : qAsConst(pathProjected))
        path << p.mapProjectionToGeo(c);


    QMapbox::Coordinates coordinates;
    for (const QGeoCoordinate &coordinate : path) {
        coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
    }
    coordinates.append(coordinates.first());  // closing the path
    QMapbox::CoordinatesCollections geometry { { coordinates } };
    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

static QMapbox::Coordinates qgeocoordinate2mapboxcoordinate(const QList<QGeoCoordinate> &crds, const bool crossesDateline, bool closed = false)
{
    QMapbox::Coordinates coordinates;
    for (const QGeoCoordinate &coordinate : crds) {
        if (!coordinates.empty() && crossesDateline && qAbs(coordinate.longitude() - coordinates.last().second) > 180.0) {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() + (coordinate.longitude() >= 0 ? -360.0 : 360.0) };
        } else {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
        }
    }
    if (closed && !coordinates.empty() && coordinates.last() != coordinates.first())
        coordinates.append(coordinates.first());  // closing the path
    return coordinates;
}

QMapbox::Feature featureFromMapPolygon(QDeclarativePolygonMapItem *mapItem)
{
    const QGeoPolygon *polygon = static_cast<const QGeoPolygon *>(&mapItem->geoShape());
    const bool crossesDateline = geoRectangleCrossesDateLine(polygon->boundingGeoRectangle());
    QMapbox::CoordinatesCollections geometry;
    QMapbox::CoordinatesCollection poly;
    QMapbox::Coordinates coordinates = qgeocoordinate2mapboxcoordinate(polygon->perimeter(), crossesDateline, true);
    poly.push_back(coordinates);
    for (int i = 0; i < polygon->holesCount(); ++i) {
        coordinates = qgeocoordinate2mapboxcoordinate(polygon->holePath(i), crossesDateline, true);
        poly.push_back(coordinates);
    }

    geometry.push_back(poly);
    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapPolyline(QDeclarativePolylineMapItem *mapItem)
{
    const QGeoPath *path = static_cast<const QGeoPath *>(&mapItem->geoShape());
    QMapbox::Coordinates coordinates;
    const bool crossesDateline = geoRectangleCrossesDateLine(path->boundingGeoRectangle());
    for (const QGeoCoordinate &coordinate : path->path()) {
        if (!coordinates.empty() && crossesDateline && qAbs(coordinate.longitude() - coordinates.last().second) > 180.0) {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() + (coordinate.longitude() >= 0 ? -360.0 : 360.0) };
        } else {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
        }
    }
    QMapbox::CoordinatesCollections geometry { { coordinates } };

    return QMapbox::Feature(QMapbox::Feature::LineStringType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapItem(QDeclarativeGeoMapItemBase *item)
{
    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
        return featureFromMapRectangle(static_cast<QDeclarativeRectangleMapItem *>(item));
    case QGeoMap::MapCircle:
        return featureFromMapCircle(static_cast<QDeclarativeCircleMapItem *>(item));
    case QGeoMap::MapPolygon:
        return featureFromMapPolygon(static_cast<QDeclarativePolygonMapItem *>(item));
    case QGeoMap::MapPolyline:
        return featureFromMapPolyline(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return QMapbox::Feature();
    }
}

QList<QByteArray> getAllPropertyNamesList(QObject *object)
{
    const QMetaObject *metaObject = object->metaObject();
    QList<QByteArray> propertyNames(object->dynamicPropertyNames());
    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        propertyNames.append(metaObject->property(i).name());
    }
    return propertyNames;
}

} // namespace


// QMaplibreGLStyleChange

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleChange::addMapParameter(QGeoMapParameter *param)
{
    static const QStringList acceptedParameterTypes = QStringList()
        << QStringLiteral("paint") << QStringLiteral("layout") << QStringLiteral("filter")
        << QStringLiteral("layer") << QStringLiteral("source") << QStringLiteral("image");

    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    switch (acceptedParameterTypes.indexOf(param->type())) {
    case -1:
        qWarning() << "Invalid value for property 'type': " + param->type();
        break;
    case 0: // paint
        changes << QMaplibreGLStyleSetPaintProperty::fromMapParameter(param);
        break;
    case 1: // layout
        changes << QMaplibreGLStyleSetLayoutProperty::fromMapParameter(param);
        break;
    case 2: // filter
        changes << QMaplibreGLStyleSetFilter::fromMapParameter(param);
        break;
    case 3: // layer
        changes << QMaplibreGLStyleAddLayer::fromMapParameter(param);
        break;
    case 4: // source
        changes << QMaplibreGLStyleAddSource::fromMapParameter(param);
        break;
    case 5: // image
        changes << QMaplibreGLStyleAddImage::fromMapParameter(param);
        break;
    }

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleChange::addMapItem(QDeclarativeGeoMapItemBase *item, const QString &before)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
    case QGeoMap::MapCircle:
    case QGeoMap::MapPolygon:
    case QGeoMap::MapPolyline:
        break;
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return changes;
    }

    QMapbox::Feature feature = featureFromMapItem(item);

    changes << QMaplibreGLStyleAddLayer::fromFeature(feature, before);
    changes << QMaplibreGLStyleAddSource::fromFeature(feature);
    changes << QMaplibreGLStyleSetPaintProperty::fromMapItem(item);
    changes << QMaplibreGLStyleSetLayoutProperty::fromMapItem(item);

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleChange::removeMapParameter(QGeoMapParameter *param)
{
    static const QStringList acceptedParameterTypes = QStringList()
        << QStringLiteral("paint") << QStringLiteral("layout") << QStringLiteral("filter")
        << QStringLiteral("layer") << QStringLiteral("source") << QStringLiteral("image");

    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    switch (acceptedParameterTypes.indexOf(param->type())) {
    case -1:
        qWarning() << "Invalid value for property 'type': " + param->type();
        break;
    case 0: // paint
    case 1: // layout
    case 2: // filter
        break;
    case 3: // layer
        changes << QSharedPointer<QMaplibreGLStyleChange>(new QMaplibreGLStyleRemoveLayer(param->property("name").toString()));
        break;
    case 4: // source
        changes << QSharedPointer<QMaplibreGLStyleChange>(new QMaplibreGLStyleRemoveSource(param->property("name").toString()));
        break;
    case 5: // image
        break;
    }

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleChange::removeMapItem(QDeclarativeGeoMapItemBase *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(new QMaplibreGLStyleRemoveLayer(id));
    changes << QSharedPointer<QMaplibreGLStyleChange>(new QMaplibreGLStyleRemoveSource(id));

    return changes;
}

// QMaplibreGLStyleSetLayoutProperty

void QMaplibreGLStyleSetLayoutProperty::apply(QMapboxGL *map)
{
    map->setLayoutProperty(m_layer, m_property, m_value);
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetLayoutProperty::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "layout");

    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    QList<QByteArray> propertyNames = getAllPropertyNamesList(param);
    for (const QByteArray &propertyName : propertyNames) {
        if (isImmutableProperty(propertyName))
            continue;

        auto layout = new QMaplibreGLStyleSetLayoutProperty();

        layout->m_value = param->property(propertyName);
        if (layout->m_value.canConvert<QJSValue>()) {
            layout->m_value = layout->m_value.value<QJSValue>().toVariant();
        }

        layout->m_layer = param->property("layer").toString();
        layout->m_property = formatPropertyName(propertyName);

        changes << QSharedPointer<QMaplibreGLStyleChange>(layout);
    }

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetLayoutProperty::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    switch (item->itemType()) {
    case QGeoMap::MapPolyline:
        changes = fromMapItem(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        break;
    }

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetLayoutProperty(getId(item), QStringLiteral("visibility"),
            item->isVisible() ? QStringLiteral("visible") : QStringLiteral("none")));

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetLayoutProperty::fromMapItem(QDeclarativePolylineMapItem *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;
    changes.reserve(2);

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetLayoutProperty(id, QStringLiteral("line-cap"), QStringLiteral("square")));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetLayoutProperty(id, QStringLiteral("line-join"), QStringLiteral("bevel")));

    return changes;
}

QMaplibreGLStyleSetLayoutProperty::QMaplibreGLStyleSetLayoutProperty(const QString& layer, const QString& property, const QVariant &value)
    : m_layer(layer), m_property(property), m_value(value)
{
}

// QMaplibreGLStyleSetPaintProperty

QMaplibreGLStyleSetPaintProperty::QMaplibreGLStyleSetPaintProperty(const QString& layer, const QString& property, const QVariant &value)
    : m_layer(layer), m_property(property), m_value(value)
{
}

void QMaplibreGLStyleSetPaintProperty::apply(QMapboxGL *map)
{
    map->setPaintProperty(m_layer, m_property, m_value);
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "paint");

    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;

    QList<QByteArray> propertyNames = getAllPropertyNamesList(param);
    for (const QByteArray &propertyName : propertyNames) {
        if (isImmutableProperty(propertyName))
            continue;

        auto paint = new QMaplibreGLStyleSetPaintProperty();

        paint->m_value = param->property(propertyName);
        if (paint->m_value.canConvert<QJSValue>()) {
            paint->m_value = paint->m_value.value<QJSValue>().toVariant();
        }

        paint->m_layer = param->property("layer").toString();
        paint->m_property = formatPropertyName(propertyName);

        changes << QSharedPointer<QMaplibreGLStyleChange>(paint);
    }

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
        return fromMapItem(static_cast<QDeclarativeRectangleMapItem *>(item));
    case QGeoMap::MapCircle:
        return fromMapItem(static_cast<QDeclarativeCircleMapItem *>(item));
    case QGeoMap::MapPolygon:
        return fromMapItem(static_cast<QDeclarativePolygonMapItem *>(item));
    case QGeoMap::MapPolyline:
        return fromMapItem(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return QList<QSharedPointer<QMaplibreGLStyleChange>>();
    }
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapItem(QDeclarativeRectangleMapItem *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapItem(QDeclarativeCircleMapItem *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapItem(QDeclarativePolygonMapItem *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMaplibreGLStyleChange>> QMaplibreGLStyleSetPaintProperty::fromMapItem(QDeclarativePolylineMapItem *item)
{
    QList<QSharedPointer<QMaplibreGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("line-opacity"), item->line()->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("line-color"), item->line()->color()));
    changes << QSharedPointer<QMaplibreGLStyleChange>(
        new QMaplibreGLStyleSetPaintProperty(id, QStringLiteral("line-width"), item->line()->width()));

    return changes;
}

// QMaplibreGLStyleAddLayer

void QMaplibreGLStyleAddLayer::apply(QMapboxGL *map)
{
    map->addLayer(m_params, m_before);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddLayer::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "layer");

    auto layer = new QMaplibreGLStyleAddLayer();

    static const QStringList layerProperties = QStringList()
        << QStringLiteral("name") << QStringLiteral("layerType") << QStringLiteral("before");

    QList<QByteArray> propertyNames = getAllPropertyNamesList(param);
    for (const QByteArray &propertyName : propertyNames) {
        if (isImmutableProperty(propertyName))
            continue;

        const QVariant value = param->property(propertyName);

        switch (layerProperties.indexOf(propertyName)) {
        case -1:
            layer->m_params[formatPropertyName(propertyName)] = value;
            break;
        case 0: // name
            layer->m_params[QStringLiteral("id")] = value;
            break;
        case 1: // layerType
            layer->m_params[QStringLiteral("type")] = value;
            break;
        case 2: // before
            layer->m_before = value.toString();
            break;
        }
    }

    return QSharedPointer<QMaplibreGLStyleChange>(layer);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddLayer::fromFeature(const QMapbox::Feature &feature, const QString &before)
{
    auto layer = new QMaplibreGLStyleAddLayer();
    layer->m_params[QStringLiteral("id")] = feature.id;
    layer->m_params[QStringLiteral("source")] = feature.id;

    switch (feature.type) {
    case QMapbox::Feature::PointType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("circle");
        break;
    case QMapbox::Feature::LineStringType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("line");
        break;
    case QMapbox::Feature::PolygonType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("fill");
        break;
    }

    layer->m_before = before;

    return QSharedPointer<QMaplibreGLStyleChange>(layer);
}


// QMaplibreGLStyleRemoveLayer

void QMaplibreGLStyleRemoveLayer::apply(QMapboxGL *map)
{
    map->removeLayer(m_id);
}

QMaplibreGLStyleRemoveLayer::QMaplibreGLStyleRemoveLayer(const QString &id) : m_id(id)
{
}


// QMaplibreGLStyleAddSource

void QMaplibreGLStyleAddSource::apply(QMapboxGL *map)
{
    map->updateSource(m_id, m_params);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddSource::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "source");

    static const QStringList acceptedSourceTypes = QStringList()
        << QStringLiteral("vector") << QStringLiteral("raster") << QStringLiteral("raster-dem") << QStringLiteral("geojson") << QStringLiteral("image");

    QString sourceType = param->property("sourceType").toString();

    auto source = new QMaplibreGLStyleAddSource();
    source->m_id = param->property("name").toString();
    source->m_params[QStringLiteral("type")] = sourceType;

    switch (acceptedSourceTypes.indexOf(sourceType)) {
    case -1:
        qWarning() << "Invalid value for property 'sourceType': " + sourceType;
        break;
    case 0: // vector
    case 1: // raster
    case 2: // raster-dem
        if (param->hasProperty("url")) {
            source->m_params[QStringLiteral("url")] = param->property("url");
        }
        if (param->hasProperty("tiles")) {
            source->m_params[QStringLiteral("tiles")] = param->property("tiles");
        }
        if (param->hasProperty("tileSize")) {
            source->m_params[QStringLiteral("tileSize")] = param->property("tileSize");
        }
        break;
    case 3: { // geojson
        auto data = param->property("data").toString();
        if (data.startsWith(':')) {
            QFile geojson(data);
            geojson.open(QIODevice::ReadOnly);
            source->m_params[QStringLiteral("data")] = geojson.readAll();
        } else {
            source->m_params[QStringLiteral("data")] = data.toUtf8();
        }
    } break;
    case 4: { // image
        source->m_params[QStringLiteral("url")] = param->property("url");
        source->m_params[QStringLiteral("coordinates")] = param->property("coordinates");
    } break;
    }

    return QSharedPointer<QMaplibreGLStyleChange>(source);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddSource::fromFeature(const QMapbox::Feature &feature)
{
    auto source = new QMaplibreGLStyleAddSource();

    source->m_id = feature.id.toString();
    source->m_params[QStringLiteral("type")] = QStringLiteral("geojson");
    source->m_params[QStringLiteral("data")] = QVariant::fromValue<QMapbox::Feature>(feature);

    return QSharedPointer<QMaplibreGLStyleChange>(source);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddSource::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    return fromFeature(featureFromMapItem(item));
}


// QMaplibreGLStyleRemoveSource

void QMaplibreGLStyleRemoveSource::apply(QMapboxGL *map)
{
    map->removeSource(m_id);
}

QMaplibreGLStyleRemoveSource::QMaplibreGLStyleRemoveSource(const QString &id) : m_id(id)
{
}


// QMaplibreGLStyleSetFilter

void QMaplibreGLStyleSetFilter::apply(QMapboxGL *map)
{
    map->setFilter(m_layer, m_filter);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleSetFilter::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "filter");

    auto filter = new QMaplibreGLStyleSetFilter();
    filter->m_layer = param->property("layer").toString();
    filter->m_filter = param->property("filter");

    return QSharedPointer<QMaplibreGLStyleChange>(filter);
}


// QMaplibreGLStyleAddImage

void QMaplibreGLStyleAddImage::apply(QMapboxGL *map)
{
    map->addImage(m_name, m_sprite);
}

QSharedPointer<QMaplibreGLStyleChange> QMaplibreGLStyleAddImage::fromMapParameter(QGeoMapParameter *param)
{
    Q_ASSERT(param->type() == "image");

    auto image = new QMaplibreGLStyleAddImage();
    image->m_name = param->property("name").toString();
    image->m_sprite = QImage(param->property("sprite").toString());

    return QSharedPointer<QMaplibreGLStyleChange>(image);
}
