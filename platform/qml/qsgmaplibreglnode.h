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

#ifndef QSGMAPLIBREGLNODE_H
#define QSGMAPLIBREGLNODE_H

#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRenderNode>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/private/qsgtexture_p.h>
#include <QtOpenGL/QOpenGLFramebufferObject>

#include <QMapboxGL>

class QGeoMapMaplibreGL;

class QSGMaplibreGLTextureNode : public QSGSimpleTextureNode
{
public:
    QSGMaplibreGLTextureNode(const QMapboxGLSettings &, const QSize &, qreal pixelRatio, QGeoMapMaplibreGL *geoMap);

    QMapboxGL* map() const;

    void resize(const QSize &size, qreal pixelRatio, QQuickWindow *window);
    void render(QQuickWindow *);

private:
    QScopedPointer<QMapboxGL> m_map;
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
};

class QSGMaplibreGLRenderNode : public QSGRenderNode
{
public:
    QSGMaplibreGLRenderNode(const QMapboxGLSettings &, const QSize &, qreal pixelRatio, QGeoMapMaplibreGL *geoMap);

    QMapboxGL* map() const;

    // QSGRenderNode
    void render(const RenderState *state) override;
    StateFlags changedStates() const override;

private:
    QScopedPointer<QMapboxGL> m_map;
};

#endif // QSGMAPLIBREGLNODE_H
