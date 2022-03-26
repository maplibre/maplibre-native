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

#ifndef QGEOMAPMAPLIBREGL_H
#define QGEOMAPMAPLIBREGL_H

#include "qgeomappingmanagerenginemaplibregl.h"
#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qgeomapparameter_p.h>

class QGeoMapMaplibreGLPrivate;

class QGeoMapMaplibreGL : public QGeoMap
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoMapMaplibreGL)

public:
    QGeoMapMaplibreGL(QGeoMappingManagerEngineMaplibreGL *engine, QObject *parent);
    virtual ~QGeoMapMaplibreGL();

    QString copyrightsStyleSheet() const override;
    void setMaplibreGLSettings(const QMapboxGLSettings &);
    void setUseFBO(bool);
    void setMapItemsBefore(const QString &);
    Capabilities capabilities() const override;

private Q_SLOTS:
    // QMaplibreGL
    void onMapChanged(QMapboxGL::MapChange);

    // QDeclarativeGeoMapItemBase
    void onMapItemPropertyChanged();
    void onMapItemSubPropertyChanged();
    void onMapItemUnsupportedPropertyChanged();
    void onMapItemGeometryChanged();

    // QGeoMapParameter
    void onParameterPropertyUpdated(QGeoMapParameter *param, const char *propertyName);

public Q_SLOTS:
    void copyrightsChanged(const QString &copyrightsHtml);

private:
    QSGNode *updateSceneGraph(QSGNode *oldNode, QQuickWindow *window) override;

    QGeoMappingManagerEngineMaplibreGL *m_engine;
};

#endif // QGEOMAPMAPLIBREGL_H
