/****************************************************************************
**
** Copyright (C) 2017 Mapbox, Inc.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQMAPLIBREGLSTYLECHANGE_P_H
#define QQMAPLIBREGLSTYLECHANGE_P_H

#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtGui/QImage>
#include <QtLocation/private/qdeclarativecirclemapitem_p.h>
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qdeclarativerectanglemapitem_p.h>
#include <QtLocation/private/qgeomapparameter_p.h>

#include <QMapboxGL>

class QMaplibreGLStyleChange
{
public:
    virtual ~QMaplibreGLStyleChange() = default;

    static QList<QSharedPointer<QMaplibreGLStyleChange>> addMapParameter(QGeoMapParameter *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> addMapItem(QDeclarativeGeoMapItemBase *, const QString &before);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> removeMapParameter(QGeoMapParameter *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> removeMapItem(QDeclarativeGeoMapItemBase *);

    virtual void apply(QMapboxGL *map) = 0;
};

class QMaplibreGLStyleSetLayoutProperty : public QMaplibreGLStyleChange
{
public:
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapParameter(QGeoMapParameter *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativePolylineMapItem *);

    QMaplibreGLStyleSetLayoutProperty() = default;
    QMaplibreGLStyleSetLayoutProperty(const QString &layer, const QString &property, const QVariant &value);

    QString m_layer;
    QString m_property;
    QVariant m_value;
};

class QMaplibreGLStyleSetPaintProperty : public QMaplibreGLStyleChange
{
public:
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapParameter(QGeoMapParameter *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativeRectangleMapItem *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativeCircleMapItem *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativePolygonMapItem *);
    static QList<QSharedPointer<QMaplibreGLStyleChange>> fromMapItem(QDeclarativePolylineMapItem *);

    QMaplibreGLStyleSetPaintProperty() = default;
    QMaplibreGLStyleSetPaintProperty(const QString &layer, const QString &property, const QVariant &value);

    QString m_layer;
    QString m_property;
    QVariant m_value;
};

class QMaplibreGLStyleAddLayer : public QMaplibreGLStyleChange
{
public:
    static QSharedPointer<QMaplibreGLStyleChange> fromMapParameter(QGeoMapParameter *);
    static QSharedPointer<QMaplibreGLStyleChange> fromFeature(const QMapbox::Feature &feature, const QString &before);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleAddLayer() = default;

    QVariantMap m_params;
    QString m_before;
};

class QMaplibreGLStyleRemoveLayer : public QMaplibreGLStyleChange
{
public:
    explicit QMaplibreGLStyleRemoveLayer(const QString &id);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleRemoveLayer() = default;

    QString m_id;
};

class QMaplibreGLStyleAddSource : public QMaplibreGLStyleChange
{
public:
    static QSharedPointer<QMaplibreGLStyleChange> fromMapParameter(QGeoMapParameter *);
    static QSharedPointer<QMaplibreGLStyleChange> fromFeature(const QMapbox::Feature &feature);
    static QSharedPointer<QMaplibreGLStyleChange> fromMapItem(QDeclarativeGeoMapItemBase *);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleAddSource() = default;

    QString m_id;
    QVariantMap m_params;
};

class QMaplibreGLStyleRemoveSource : public QMaplibreGLStyleChange
{
public:
    explicit QMaplibreGLStyleRemoveSource(const QString &id);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleRemoveSource() = default;

    QString m_id;
};

class QMaplibreGLStyleSetFilter : public QMaplibreGLStyleChange
{
public:
    static QSharedPointer<QMaplibreGLStyleChange> fromMapParameter(QGeoMapParameter *);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleSetFilter() = default;

    QString m_layer;
    QVariant m_filter;
};

class QMaplibreGLStyleAddImage : public QMaplibreGLStyleChange
{
public:
    static QSharedPointer<QMaplibreGLStyleChange> fromMapParameter(QGeoMapParameter *);

    void apply(QMapboxGL *map) override;

private:
    QMaplibreGLStyleAddImage() = default;

    QString m_name;
    QImage m_sprite;
};

#endif // QQMAPLIBREGLSTYLECHANGE_P_H
