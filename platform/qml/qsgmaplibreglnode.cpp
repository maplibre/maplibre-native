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

#include "qsgmaplibreglnode.h"
#include "qgeomapmaplibregl.h"

#if QT_HAS_INCLUDE(<QtQuick/private/qsgplaintexture_p.h>)
#include <QtQuick/private/qsgplaintexture_p.h>
#endif

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QSGRendererInterface>
#include <QQuickOpenGLUtils>

// QSGMaplibreGLTextureNode

static const QSize minTextureSize = QSize(64, 64);

QSGMaplibreGLTextureNode::QSGMaplibreGLTextureNode(const QMapboxGLSettings &settings, const QSize &size, qreal pixelRatio, QGeoMapMaplibreGL *geoMap)
        : QSGSimpleTextureNode()
{
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    setFiltering(QSGTexture::Linear);

    m_map.reset(new QMapboxGL(nullptr, settings, size.expandedTo(minTextureSize), pixelRatio));

    QObject::connect(m_map.data(), &QMapboxGL::needsRendering, geoMap, &QGeoMap::sgNodeChanged);
    QObject::connect(m_map.data(), &QMapboxGL::copyrightsChanged, geoMap,
            static_cast<void (QGeoMap::*)(const QString &)>(&QGeoMapMaplibreGL::copyrightsChanged));
}

void QSGMaplibreGLTextureNode::resize(const QSize &size, qreal pixelRatio, QQuickWindow *window)
{
    const QSize& minSize = size.expandedTo(minTextureSize);
    const QSize fbSize = minSize * pixelRatio;
    m_map->resize(minSize);

    m_fbo.reset(new QOpenGLFramebufferObject(fbSize, QOpenGLFramebufferObject::CombinedDepthStencil));
    m_map->setFramebufferObject(m_fbo->handle(), fbSize);

    // There is no elegant way to set new frame buffer object to already created texture.
    // So, a quick hack-solution is to re-create the texture as well, every time the size changes.
    setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(m_fbo->texture(), window, fbSize, QQuickWindow::TextureHasAlphaChannel));
    setOwnsTexture(true);

    setRect(QRectF(QPointF(), minSize));
    markDirty(QSGNode::DirtyGeometry);

    qDebug() << "MBGL:" << "resize";
}

void QSGMaplibreGLTextureNode::render(QQuickWindow *window)
{
    window->beginExternalCommands();

    QOpenGLFunctions *f = static_cast<QOpenGLContext*>(window->rendererInterface()->getResource(window, QSGRendererInterface::OpenGLContextResource))->functions();

    f->glViewport(0, 0, m_fbo->width(), m_fbo->height());

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_fbo->bind();

    f->glClearColor(0.f, 0.f, 0.f, 0.f);
    f->glColorMask(true, true, true, true);
    f->glClear(GL_COLOR_BUFFER_BIT);

    m_map->render();
    m_fbo->release();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    window->endExternalCommands();
    QQuickOpenGLUtils::resetOpenGLState();
    markDirty(QSGNode::DirtyMaterial);

    qDebug() << "MBGL:" << "render window";
}

QMapboxGL* QSGMaplibreGLTextureNode::map() const
{
    return m_map.data();
}

// QSGMaplibreGLRenderNode

QSGMaplibreGLRenderNode::QSGMaplibreGLRenderNode(const QMapboxGLSettings &settings, const QSize &size, qreal pixelRatio, QGeoMapMaplibreGL *geoMap)
        : QSGRenderNode()
{
    m_map.reset(new QMapboxGL(nullptr, settings, size, pixelRatio));
    QObject::connect(m_map.data(), &QMapboxGL::needsRendering, geoMap, &QGeoMap::sgNodeChanged);
    QObject::connect(m_map.data(), &QMapboxGL::copyrightsChanged, geoMap,
            static_cast<void (QGeoMap::*)(const QString &)>(&QGeoMapMaplibreGL::copyrightsChanged));
}

QMapboxGL* QSGMaplibreGLRenderNode::map() const
{
    return m_map.data();
}

void QSGMaplibreGLRenderNode::render(const RenderState *state)
{
    // QMapboxGL assumes we've prepared the viewport prior to render().
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
    f->glScissor(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
    f->glEnable(GL_SCISSOR_TEST);

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_map->render();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    qDebug() << "MBGL:" << "render state";
}

QSGRenderNode::StateFlags QSGMaplibreGLRenderNode::changedStates() const
{
    return QSGRenderNode::DepthState
         | QSGRenderNode::StencilState
         | QSGRenderNode::ScissorState
         | QSGRenderNode::ColorState
         | QSGRenderNode::BlendState
         | QSGRenderNode::ViewportState
         | QSGRenderNode::RenderTargetState;
}
