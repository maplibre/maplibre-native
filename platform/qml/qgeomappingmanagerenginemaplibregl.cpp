/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qgeomappingmanagerenginemaplibregl.h"
#include "qgeomapmaplibregl.h"

#include <QtCore/qstandardpaths.h>
#include <QtLocation/private/qabstractgeotilecache_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>

#include <QDir>

QT_BEGIN_NAMESPACE

extern char developmentToken[];

QGeoMappingManagerEngineMaplibreGL::QGeoMappingManagerEngineMaplibreGL(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoMappingManagerEngine()
{
    *error = QGeoServiceProvider::NoError;
    errorString->clear();

    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(20.0);
    cameraCaps.setTileSize(512);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(60);
    cameraCaps.setMinimumFieldOfView(36.87);
    cameraCaps.setMaximumFieldOfView(36.87);
    setCameraCapabilities(cameraCaps);

    QList<QGeoMapType> mapTypes;
    int mapId = 0;
    const QByteArray pluginName = "maplibregl";

    if (parameters.contains(QStringLiteral("maplibregl.settings_template"))) {
        auto settings_template = parameters.value(QStringLiteral("maplibregl.settings_template")).toString();
        if (settings_template == "maptiler"){
            m_settings.resetToTemplate(QMapboxGLSettings::MapTilerSettings);
        }else if (settings_template == "mapbox"){
            m_settings.resetToTemplate(QMapboxGLSettings::MapboxSettings);
        }
    }

    if (parameters.contains(QStringLiteral("maplibregl.api_base_url"))) {
        const QString apiBaseUrl = parameters.value(QStringLiteral("maplibregl.api_base_url")).toString();
        m_settings.setApiBaseUrl(apiBaseUrl);
    }

    QVariantMap metadata;
    metadata["isHTTPS"] = true;

    for (auto &style: m_settings.defaultStyles()){
        mapTypes << QGeoMapType(QGeoMapType::StreetMap, style.first,
                style.second, false, false, ++mapId, pluginName, cameraCaps, metadata);
    }

    if (parameters.contains(QStringLiteral("maplibregl.mapping.additional_style_urls"))) {
        const QString ids = parameters.value(QStringLiteral("maplibregl.mapping.additional_style_urls")).toString();
        const QStringList idList = ids.split(',', Qt::SkipEmptyParts);

        for (auto it = idList.crbegin(), end = idList.crend(); it != end; ++it) {
            if ((*it).isEmpty())
                continue;
            if ((*it).startsWith(QStringLiteral("http:")))
                metadata["isHTTPS"] = false;
            else
                metadata["isHTTPS"] = true;

            mapTypes.prepend(QGeoMapType(QGeoMapType::CustomMap, *it,
                    tr("User provided style"), false, false, ++mapId, pluginName, cameraCaps, metadata));
        }
    }

    setSupportedMapTypes(mapTypes);

    if (parameters.contains(QStringLiteral("maplibregl.access_token"))) {
        m_settings.setApiKey(parameters.value(QStringLiteral("maplibregl.access_token")).toString());
    }else{
        if (m_settings.apiBaseUrl() == "https://api.mapbox.com"){
            // If the access token is not set, use the development access token.
            // This will only affect mapbox:// styles.
            m_settings.setApiKey(developmentToken);
        }
    }

    bool memoryCache = false;
    if (parameters.contains(QStringLiteral("maplibregl.mapping.cache.memory"))) {
        memoryCache = parameters.value(QStringLiteral("maplibregl.mapping.cache.memory")).toBool();
        m_settings.setCacheDatabasePath(QStringLiteral(":memory:"));
    }

    QString cacheDirectory;
    if (parameters.contains(QStringLiteral("maplibregl.mapping.cache.directory"))) {
        cacheDirectory = parameters.value(QStringLiteral("maplibregl.mapping.cache.directory")).toString();
    } else {
        cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QStringLiteral("maplibregl/");
    }

    if (!memoryCache && QDir::root().mkpath(cacheDirectory)) {
        m_settings.setCacheDatabasePath(cacheDirectory + "/maplibregl.db");
    }

    if (parameters.contains(QStringLiteral("maplibregl.mapping.cache.size"))) {
        bool ok = false;
        int cacheSize = parameters.value(QStringLiteral("maplibregl.mapping.cache.size")).toString().toInt(&ok);

        if (ok)
            m_settings.setCacheDatabaseMaximumSize(cacheSize);
    }

    if (parameters.contains(QStringLiteral("maplibregl.mapping.use_fbo"))) {
        m_useFBO = parameters.value(QStringLiteral("maplibregl.mapping.use_fbo")).toBool();
    }

    if (parameters.contains(QStringLiteral("maplibregl.mapping.items.insert_before"))) {
        m_mapItemsBefore = parameters.value(QStringLiteral("maplibregl.mapping.items.insert_before")).toString();
    }

    engineInitialized();
}

QGeoMappingManagerEngineMaplibreGL::~QGeoMappingManagerEngineMaplibreGL()
{
}

QGeoMap *QGeoMappingManagerEngineMaplibreGL::createMap()
{
    QGeoMapMaplibreGL* map = new QGeoMapMaplibreGL(this, 0);
    map->setMaplibreGLSettings(m_settings);
    map->setUseFBO(m_useFBO);
    map->setMapItemsBefore(m_mapItemsBefore);

    return map;
}

QT_END_NAMESPACE
