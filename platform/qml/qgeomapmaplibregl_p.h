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

#ifndef QGEOMAPMAPLIBREGL_P_H
#define QGEOMAPMAPLIBREGL_P_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtCore/QRectF>
#include <QtLocation/private/qgeomap_p_p.h>
#include <QtLocation/private/qgeomapparameter_p.h>

class QMapboxGL;
class QMaplibreGLStyleChange;

class QGeoMapMaplibreGLPrivate : public QGeoMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoMapMaplibreGL)

public:
    QGeoMapMaplibreGLPrivate(QGeoMappingManagerEngineMaplibreGL *engine);

    ~QGeoMapMaplibreGLPrivate();

    QSGNode *updateSceneGraph(QSGNode *oldNode, QQuickWindow *window);

    void addParameter(QGeoMapParameter *param) override;
    void removeParameter(QGeoMapParameter *param) override;

    QGeoMap::ItemTypes supportedMapItemTypes() const override;
    void addMapItem(QDeclarativeGeoMapItemBase *item) override;
    void removeMapItem(QDeclarativeGeoMapItemBase *item) override;

    /* Data members */
    enum SyncState : int {
        NoSync = 0,
        ViewportSync    = 1 << 0,
        CameraDataSync  = 1 << 1,
        MapTypeSync     = 1 << 2,
        VisibleAreaSync = 1 << 3
    };
    Q_DECLARE_FLAGS(SyncStates, SyncState);

    QMapboxGLSettings m_settings;
    bool m_useFBO = true;
    bool m_developmentMode = false;
    QString m_mapItemsBefore;

    QTimer m_refresh;
    bool m_shouldRefresh = true;
    bool m_warned = false;
    bool m_threadedRendering = false;
    bool m_styleLoaded = false;

    SyncStates m_syncState = NoSync;

    QList<QSharedPointer<QMaplibreGLStyleChange>> m_styleChanges;

protected:
    void changeViewportSize(const QSize &size) override;
    void changeCameraData(const QGeoCameraData &oldCameraData) override;
    void changeActiveMapType(const QGeoMapType mapType) override;

    void setVisibleArea(const QRectF &visibleArea) override;
    QRectF visibleArea() const override;

private:
    Q_DISABLE_COPY(QGeoMapMaplibreGLPrivate);

    void syncStyleChanges(QMapboxGL *map);
    void threadedRenderingHack(QQuickWindow *window, QMapboxGL *map);

    QRectF m_visibleArea;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoMapMaplibreGLPrivate::SyncStates)

#endif // QGEOMAPMAPLIBREGL_P_H
