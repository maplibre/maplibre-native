/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qgeomapmaplibregl.h"
#include "qgeomapmaplibregl_p.h"
#include "qsgmaplibreglnode.h"
#include "qmaplibreglstylechange_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtOpenGL/QOpenGLFramebufferObject>
#include <QtLocation/private/qdeclarativecirclemapitem_p.h>
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qdeclarativerectanglemapitem_p.h>
#include <QtLocation/private/qgeomapparameter_p.h>
#include <QtLocation/private/qgeoprojection_p.h>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGImageNode>
#include <QtQuick/private/qsgtexture_p.h>
#include <QtQuick/private/qsgcontext_p.h> // for debugging the context name

#include <QMapboxGL>

#include <cmath>

// FIXME: Expose from Mapbox GL constants
#define MBGL_TILE_SIZE 512.0

// WARNING! The development token is subject to Mapbox Terms of Services
// and must not be used in production.
char developmentToken[] =
    "pk.eyJ1IjoicXRzZGsiLCJhIjoiY2l5azV5MHh5MDAwdTMybzBybjUzZnhxYSJ9.9rfbeqPjX2BusLRDXHCOBA";

namespace {

static const double invLog2 = 1.0 / std::log(2.0);

static double zoomLevelFrom256(double zoomLevelFor256, double tileSize)
{
    return std::log(std::pow(2.0, zoomLevelFor256) * 256.0 / tileSize) * invLog2;
}

} // namespace

QGeoMapMaplibreGLPrivate::QGeoMapMaplibreGLPrivate(QGeoMappingManagerEngineMaplibreGL *engine)
    : QGeoMapPrivate(engine, new QGeoProjectionWebMercator)
{
}

QGeoMapMaplibreGLPrivate::~QGeoMapMaplibreGLPrivate()
{
}

QSGNode *QGeoMapMaplibreGLPrivate::updateSceneGraph(QSGNode *node, QQuickWindow *window)
{
    Q_Q(QGeoMapMaplibreGL);

    if (m_viewportSize.isEmpty()) {
        delete node;
        return 0;
    }

    QMapboxGL *map = 0;
    if (!node) {
        QOpenGLContext *currentCtx = QOpenGLContext::currentContext();
        if (!currentCtx) {
            qWarning("QOpenGLContext is NULL!");
            qWarning() << "You are running on QSG backend " << QSGContext::backend();
            qWarning("The MaplibreGL plugin works with both Desktop and ES 2.0+ OpenGL versions.");
            qWarning("Verify that your Qt is built with OpenGL, and what kind of OpenGL.");
            qWarning("To force using a specific OpenGL version, check QSurfaceFormat::setRenderableType and QSurfaceFormat::setDefaultFormat");

            return node;
        }
        if (m_useFBO) {
            QSGMaplibreGLTextureNode *mbglNode = new QSGMaplibreGLTextureNode(m_settings, m_viewportSize, window->devicePixelRatio(), q);
            QObject::connect(mbglNode->map(), &QMapboxGL::mapChanged, q, &QGeoMapMaplibreGL::onMapChanged);
            m_syncState = MapTypeSync | CameraDataSync | ViewportSync | VisibleAreaSync;
            node = mbglNode;
        } else {
            QSGMaplibreGLRenderNode *mbglNode = new QSGMaplibreGLRenderNode(m_settings, m_viewportSize, window->devicePixelRatio(), q);
            QObject::connect(mbglNode->map(), &QMapboxGL::mapChanged, q, &QGeoMapMaplibreGL::onMapChanged);
            m_syncState = MapTypeSync | CameraDataSync | ViewportSync | VisibleAreaSync;
            node = mbglNode;
        }
    }
    map = (m_useFBO) ? static_cast<QSGMaplibreGLTextureNode *>(node)->map()
                     : static_cast<QSGMaplibreGLRenderNode *>(node)->map();

    if (m_syncState & MapTypeSync) {
        m_developmentMode = m_activeMapType.name().startsWith("mapbox://")
            && m_settings.apiKey() == developmentToken;

        map->setStyleUrl(m_activeMapType.name());
    }

    if (m_syncState & VisibleAreaSync) {
        if (m_visibleArea.isEmpty()) {
            map->setMargins(QMargins());
        } else {
            QMargins margins(m_visibleArea.x(),                                                     // left
                             m_visibleArea.y(),                                                     // top
                             m_viewportSize.width() - m_visibleArea.width() - m_visibleArea.x(),    // right
                             m_viewportSize.height() - m_visibleArea.height() - m_visibleArea.y()); // bottom
            map->setMargins(margins);
        }
    }

    if (m_syncState & CameraDataSync || m_syncState & VisibleAreaSync) {
        map->setZoom(zoomLevelFrom256(m_cameraData.zoomLevel() , MBGL_TILE_SIZE));
        map->setBearing(m_cameraData.bearing());
        map->setPitch(m_cameraData.tilt());

        QGeoCoordinate coordinate = m_cameraData.center();
        map->setCoordinate(QMapbox::Coordinate(coordinate.latitude(), coordinate.longitude()));
    }

    if (m_syncState & ViewportSync) {
        if (m_useFBO) {
            static_cast<QSGMaplibreGLTextureNode *>(node)->resize(m_viewportSize, window->devicePixelRatio(), window);
        } else {
            map->resize(m_viewportSize);
        }
    }

    if (m_styleLoaded) {
        syncStyleChanges(map);
    }

    if (m_useFBO) {
        static_cast<QSGMaplibreGLTextureNode *>(node)->render(window);
    }

    threadedRenderingHack(window, map);

    m_syncState = NoSync;

    return node;
}

void QGeoMapMaplibreGLPrivate::addParameter(QGeoMapParameter *param)
{
    Q_Q(QGeoMapMaplibreGL);

    QObject::connect(param, &QGeoMapParameter::propertyUpdated, q,
        &QGeoMapMaplibreGL::onParameterPropertyUpdated);

    if (m_styleLoaded) {
        m_styleChanges << QMaplibreGLStyleChange::addMapParameter(param);
        emit q->sgNodeChanged();
    }
}

void QGeoMapMaplibreGLPrivate::removeParameter(QGeoMapParameter *param)
{
    Q_Q(QGeoMapMaplibreGL);

    q->disconnect(param);

    if (m_styleLoaded) {
        m_styleChanges << QMaplibreGLStyleChange::removeMapParameter(param);
        emit q->sgNodeChanged();
    }
}

QGeoMap::ItemTypes QGeoMapMaplibreGLPrivate::supportedMapItemTypes() const
{
    return QGeoMap::MapRectangle | QGeoMap::MapCircle | QGeoMap::MapPolygon | QGeoMap::MapPolyline;
}

void QGeoMapMaplibreGLPrivate::addMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_Q(QGeoMapMaplibreGL);

    switch (item->itemType()) {
    case QGeoMap::NoItem:
    case QGeoMap::MapQuickItem:
    case QGeoMap::CustomMapItem:
        return;
    case QGeoMap::MapRectangle: {
        QDeclarativeRectangleMapItem *mapItem = static_cast<QDeclarativeRectangleMapItem *>(item);
        QObject::connect(mapItem, &QQuickItem::visibleChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeRectangleMapItem::bottomRightChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem, &QDeclarativeRectangleMapItem::topLeftChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem, &QDeclarativeRectangleMapItem::colorChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::colorChanged, q, &QGeoMapMaplibreGL::onMapItemSubPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::widthChanged, q, &QGeoMapMaplibreGL::onMapItemUnsupportedPropertyChanged);
    } break;
    case QGeoMap::MapCircle: {
        QDeclarativeCircleMapItem *mapItem = static_cast<QDeclarativeCircleMapItem *>(item);
        QObject::connect(mapItem, &QQuickItem::visibleChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeCircleMapItem::centerChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem, &QDeclarativeCircleMapItem::radiusChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem, &QDeclarativeCircleMapItem::colorChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::colorChanged, q, &QGeoMapMaplibreGL::onMapItemSubPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::widthChanged, q, &QGeoMapMaplibreGL::onMapItemUnsupportedPropertyChanged);
    } break;
    case QGeoMap::MapPolygon: {
        QDeclarativePolygonMapItem *mapItem = static_cast<QDeclarativePolygonMapItem *>(item);
        QObject::connect(mapItem, &QQuickItem::visibleChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativePolygonMapItem::pathChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem, &QDeclarativePolygonMapItem::colorChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::colorChanged, q, &QGeoMapMaplibreGL::onMapItemSubPropertyChanged);
        QObject::connect(mapItem->border(), &QDeclarativeMapLineProperties::widthChanged, q, &QGeoMapMaplibreGL::onMapItemUnsupportedPropertyChanged);
    } break;
    case QGeoMap::MapPolyline: {
        QDeclarativePolylineMapItem *mapItem = static_cast<QDeclarativePolylineMapItem *>(item);
        QObject::connect(mapItem, &QQuickItem::visibleChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);
        QObject::connect(mapItem, &QDeclarativePolylineMapItem::pathChanged, q, &QGeoMapMaplibreGL::onMapItemGeometryChanged);
        QObject::connect(mapItem->line(), &QDeclarativeMapLineProperties::colorChanged, q, &QGeoMapMaplibreGL::onMapItemSubPropertyChanged);
        QObject::connect(mapItem->line(), &QDeclarativeMapLineProperties::widthChanged, q, &QGeoMapMaplibreGL::onMapItemSubPropertyChanged);
    } break;
    }

    QObject::connect(item, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged, q, &QGeoMapMaplibreGL::onMapItemPropertyChanged);

    m_styleChanges << QMaplibreGLStyleChange::addMapItem(item, m_mapItemsBefore);

    emit q->sgNodeChanged();
}

void QGeoMapMaplibreGLPrivate::removeMapItem(QDeclarativeGeoMapItemBase *item)
{
    Q_Q(QGeoMapMaplibreGL);

    switch (item->itemType()) {
    case QGeoMap::NoItem:
    case QGeoMap::MapQuickItem:
    case QGeoMap::CustomMapItem:
        return;
    case QGeoMap::MapRectangle:
        q->disconnect(static_cast<QDeclarativeRectangleMapItem *>(item)->border());
        break;
    case QGeoMap::MapCircle:
        q->disconnect(static_cast<QDeclarativeCircleMapItem *>(item)->border());
        break;
    case QGeoMap::MapPolygon:
        q->disconnect(static_cast<QDeclarativePolygonMapItem *>(item)->border());
        break;
    case QGeoMap::MapPolyline:
        q->disconnect(static_cast<QDeclarativePolylineMapItem *>(item)->line());
        break;
    }

    q->disconnect(item);

    m_styleChanges << QMaplibreGLStyleChange::removeMapItem(item);

    emit q->sgNodeChanged();
}

void QGeoMapMaplibreGLPrivate::changeViewportSize(const QSize &)
{
    Q_Q(QGeoMapMaplibreGL);

    m_syncState = m_syncState | ViewportSync;
    emit q->sgNodeChanged();
}

void QGeoMapMaplibreGLPrivate::changeCameraData(const QGeoCameraData &)
{
    Q_Q(QGeoMapMaplibreGL);

    m_syncState = m_syncState | CameraDataSync;
    emit q->sgNodeChanged();
}

void QGeoMapMaplibreGLPrivate::changeActiveMapType(const QGeoMapType)
{
    Q_Q(QGeoMapMaplibreGL);

    m_syncState = m_syncState | MapTypeSync;
    emit q->sgNodeChanged();
}

void QGeoMapMaplibreGLPrivate::setVisibleArea(const QRectF &visibleArea)
{
    Q_Q(QGeoMapMaplibreGL);
    const QRectF va = clampVisibleArea(visibleArea);
    if (va == m_visibleArea)
        return;

    m_visibleArea = va;
    m_geoProjection->setVisibleArea(va);

    m_syncState = m_syncState | VisibleAreaSync;
    emit q->sgNodeChanged();
}

QRectF QGeoMapMaplibreGLPrivate::visibleArea() const
{
    return m_visibleArea;
}

void QGeoMapMaplibreGLPrivate::syncStyleChanges(QMapboxGL *map)
{
    for (const auto& change : m_styleChanges) {
        change->apply(map);
    }

    m_styleChanges.clear();
}

void QGeoMapMaplibreGLPrivate::threadedRenderingHack(QQuickWindow *window, QMapboxGL *map)
{
    // FIXME: Optimal support for threaded rendering needs core changes
    // in Mapbox GL Native. Meanwhile we need to set a timer to update
    // the map until all the resources are loaded, which is not exactly
    // battery friendly, because might trigger more paints than we need.
    if (!m_warned) {
        m_threadedRendering = static_cast<QOpenGLContext*>(window->rendererInterface()->getResource(window, QSGRendererInterface::OpenGLContextResource))->thread() != QCoreApplication::instance()->thread();

        if (m_threadedRendering) {
            qWarning() << "Threaded rendering is not optimal in the Mapbox GL plugin.";
        }

        m_warned = true;
    }

    if (m_threadedRendering) {
        if (!map->isFullyLoaded()) {
            QMetaObject::invokeMethod(&m_refresh, "start", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(&m_refresh, "stop", Qt::QueuedConnection);
        }
    }
}

/*
 * QGeoMapMaplibreGL implementation
 */

QGeoMapMaplibreGL::QGeoMapMaplibreGL(QGeoMappingManagerEngineMaplibreGL *engine, QObject *parent)
    :   QGeoMap(*new QGeoMapMaplibreGLPrivate(engine), parent), m_engine(engine)
{
    Q_D(QGeoMapMaplibreGL);

    connect(&d->m_refresh, &QTimer::timeout, this, &QGeoMap::sgNodeChanged);
    d->m_refresh.setInterval(250);
}

QGeoMapMaplibreGL::~QGeoMapMaplibreGL()
{
}

QString QGeoMapMaplibreGL::copyrightsStyleSheet() const
{
    return QStringLiteral("* { vertical-align: middle; font-weight: normal }");
}

void QGeoMapMaplibreGL::setMaplibreGLSettings(const QMapboxGLSettings& settings)
{
    Q_D(QGeoMapMaplibreGL);

    d->m_settings = settings;
}

void QGeoMapMaplibreGL::setUseFBO(bool useFBO)
{
    Q_D(QGeoMapMaplibreGL);
    d->m_useFBO = useFBO;
}

void QGeoMapMaplibreGL::setMapItemsBefore(const QString &before)
{
    Q_D(QGeoMapMaplibreGL);
    d->m_mapItemsBefore = before;
}

QGeoMap::Capabilities QGeoMapMaplibreGL::capabilities() const
{
    return Capabilities(SupportsVisibleRegion
                        | SupportsSetBearing
                        | SupportsAnchoringCoordinate
                        | SupportsVisibleArea );
}

QSGNode *QGeoMapMaplibreGL::updateSceneGraph(QSGNode *oldNode, QQuickWindow *window)
{
    Q_D(QGeoMapMaplibreGL);
    return d->updateSceneGraph(oldNode, window);
}

void QGeoMapMaplibreGL::onMapChanged(QMapboxGL::MapChange change)
{
    Q_D(QGeoMapMaplibreGL);

    if (change == QMapboxGL::MapChangeDidFinishLoadingStyle || change == QMapboxGL::MapChangeDidFailLoadingMap) {
        d->m_styleLoaded = true;
    } else if (change == QMapboxGL::MapChangeWillStartLoadingMap) {
        d->m_styleLoaded = false;
        d->m_styleChanges.clear();

        for (QDeclarativeGeoMapItemBase *item : d->m_mapItems)
            d->m_styleChanges << QMaplibreGLStyleChange::addMapItem(item, d->m_mapItemsBefore);

        for (QGeoMapParameter *param : d->m_mapParameters)
            d->m_styleChanges << QMaplibreGLStyleChange::addMapParameter(param);
    }
}

void QGeoMapMaplibreGL::onMapItemPropertyChanged()
{
    Q_D(QGeoMapMaplibreGL);

    QDeclarativeGeoMapItemBase *item = static_cast<QDeclarativeGeoMapItemBase *>(sender());
    d->m_styleChanges << QMaplibreGLStyleSetPaintProperty::fromMapItem(item);
    d->m_styleChanges << QMaplibreGLStyleSetLayoutProperty::fromMapItem(item);

    emit sgNodeChanged();
}

void QGeoMapMaplibreGL::onMapItemSubPropertyChanged()
{
    Q_D(QGeoMapMaplibreGL);

    QDeclarativeGeoMapItemBase *item = static_cast<QDeclarativeGeoMapItemBase *>(sender()->parent());
    d->m_styleChanges << QMaplibreGLStyleSetPaintProperty::fromMapItem(item);

    emit sgNodeChanged();
}

void QGeoMapMaplibreGL::onMapItemUnsupportedPropertyChanged()
{
    // TODO https://bugreports.qt.io/browse/QTBUG-58872
    qWarning() << "Unsupported property for managed Map item";
}

void QGeoMapMaplibreGL::onMapItemGeometryChanged()
{
    Q_D(QGeoMapMaplibreGL);

    QDeclarativeGeoMapItemBase *item = static_cast<QDeclarativeGeoMapItemBase *>(sender());
    d->m_styleChanges << QMaplibreGLStyleAddSource::fromMapItem(item);

    emit sgNodeChanged();
}

void QGeoMapMaplibreGL::onParameterPropertyUpdated(QGeoMapParameter *param, const char *)
{
    Q_D(QGeoMapMaplibreGL);

    d->m_styleChanges.append(QMaplibreGLStyleChange::addMapParameter(param));

    emit sgNodeChanged();
}

void QGeoMapMaplibreGL::copyrightsChanged(const QString &copyrightsHtml)
{
    Q_D(QGeoMapMaplibreGL);

    QString copyrightsHtmlFinal = copyrightsHtml;

    if (d->m_developmentMode) {
        copyrightsHtmlFinal.prepend("<a href='https://www.mapbox.com/pricing'>"
            + tr("Development access token, do not use in production.") + "</a> - ");
    }

    if (d->m_activeMapType.name().startsWith("mapbox://")) {
        copyrightsHtmlFinal = "<table><tr><th><img src='qrc:/maplibregl/mapbox_logo.png'/></th><th>"
            + copyrightsHtmlFinal + "</th></tr></table>";
    }

    QGeoMap::copyrightsChanged(copyrightsHtmlFinal);
}
